// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <cassert>
#include <thread>
#include <unordered_map>
#include "threadargs.h"

#include "pthread_u.h"

// Host maintains a map of enclave key to host thread ID
static std::unordered_map<uint64_t, std::atomic<std::thread::id>>
    _enclave_host_id_map;
// Host maintains a map of thread id to the thread object
static std::unordered_map<std::thread::id, std::thread> _host_id_thread_map;

void host_exit(int arg)
{
    // Ensure all the threads terminate before the exit
    for (auto& pair : _host_id_thread_map)
    {
        pair.second.join();
    }
    exit(arg);
}

static atomic_flag_lock _host_lock;
struct thread_args_t
{
    oe_enclave_t* enclave;
    uint64_t enc_key;
};

void* host_enclave_thread(void* args)
{
    thread_args_t* thread_args = reinterpret_cast<thread_args_t*>(args);
    // need to cache the values for enc_key and enclave now before _host_lock
    // is unlocked after assigning the thread_id to the _enclave_host_id_map
    // because args is a local variable in the calling method which may exit
    // at any time after _host_lock is unlocked which may cause a segfault
    uint64_t enc_key = thread_args->enc_key;
    oe_enclave_t* enclave = thread_args->enclave;
    std::thread::id thread_id;

    {
        // Using atomic_thread_host_id_map lock to ensure the mapping is updated
        // before the thread_id lookup
        atomic_lock lock(_host_lock);

        std::thread::id thread_id = std::this_thread::get_id();
        assert(
            _host_id_thread_map.find(thread_id) != _host_id_thread_map.end());

        // Populate the enclave_host_id map with the thread_id
        _enclave_host_id_map[enc_key] = thread_id;
    }

    // Launch the thread
    oe_result_t result = enc_enclave_thread(enclave, enc_key);
    assert(result == OE_OK);

    return NULL;
}

void host_create_thread(uint64_t enc_key, oe_enclave_t* enclave)
{
    thread_args_t thread_args = {enclave, enc_key};
    std::thread::id thread_id;
    const std::atomic<std::thread::id>* mapped_thread_id;

    {
        // Using atomic locks to protect the enclave_host_id_map
        // and update the _host_id_thread_map upon the thread creation
        atomic_lock lock(_host_lock);
        mapped_thread_id = &_enclave_host_id_map[enc_key];

        // New Thread is created and executes host_enclave_thread
        std::thread t = std::thread(host_enclave_thread, &thread_args);

        thread_id = t.get_id();
        _host_id_thread_map[thread_id] = std::move(t);
    }

    // Main host thread waits for the enclave id to host id mapping to be
    // updated
    while (*mapped_thread_id == std::thread::id())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Sanity check
    if (thread_id != *mapped_thread_id)
    {
        abort();
    }
}

int host_join_thread(uint64_t enc_key)
{
    int ret_val = 0;
    std::thread::id thread_id;

    // Find the thread_id from the enclave_host_id_map using the enc_key
    {
        // Using atomic locks to protect the enclave_host_id_map
        atomic_lock lock(_host_lock);
        const auto it = _enclave_host_id_map.find(enc_key);
        if (it != _enclave_host_id_map.end())
        {
            thread_id = it->second;
            lock.unlock();

            auto& t = _host_id_thread_map[thread_id];
            assert(t.joinable());
            t.join();

            // Update the shared memory only after join
            {
                // Delete the enclave_host_id mapping as the thread_id may be
                // reused in future
                lock.lock();
                _enclave_host_id_map.erase(enc_key);
            }
        }
        else
        {
            abort();
        }
    }

    return ret_val;
}

int host_detach_thread(uint64_t enc_key)
{
    // Find the thread_id from the enclave_host_id_map using the enc_key

    // Using atomic locks to protect the enclave_host_id_map
    atomic_lock lock(_host_lock);
    const auto it = _enclave_host_id_map.find(enc_key);
    if (it != _enclave_host_id_map.end())
    {
        std::thread::id thread_id = it->second;
        lock.unlock();

        auto& t = _host_id_thread_map[thread_id];
        t.detach();

        {
            // Delete the _enclave_host_id mapping as the host thread_id may be
            // reused in future
            lock.lock();
            _enclave_host_id_map.erase(enc_key);
        }
    }
    else
    {
        abort();
    }
    return 0;
}
