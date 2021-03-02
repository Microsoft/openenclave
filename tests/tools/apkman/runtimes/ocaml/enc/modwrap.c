// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <stdio.h>
#include <string.h>
#include <caml/mlvalues.h>
#include <caml/callback.h>

void ocaml_main(char** argv)
{
    caml_startup_pooled(argv);

    static const value * _closure = NULL;
    if (_closure == NULL)
	_closure = caml_named_value("ocaml_main");

    caml_callback(*_closure, Val_unit);

    caml_shutdown();
}

