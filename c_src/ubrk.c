/* -------------------------------------------------------------------
 *
 * icu4e: Erlang NIF wrappers for ICU
 *
 * Copyright (c) 2010 Basho Technologies, Inc.  All Rights Reserved.
 *
 * This file is provided to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License.  You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 * -------------------------------------------------------------------
 */

/*
 * NIF implementations for ubrk.erl
 *
 * All binaries passed to these functions should be UTF16-encoded,
 * in the native endian.
 */
#include "erl_nif_compat.h"
#include "icu4e.h"
#include "unicode/utypes.h"
#include "unicode/ustring.h"
#include "unicode/ubrk.h"

/* Prototypes */
ERL_NIF_TERM ubrk_words(ErlNifEnv* env, int argc,
                         const ERL_NIF_TERM argv[]);

static ErlNifFunc nif_funcs[] =
{
    {"words", 1, ubrk_words}
};

/*
 * Split the input string on word boundaries, according to the
 * UBRK_WORD break iterator.
 * Inputs:
 *   0: binary(), the string to split
 * Outputs (one of):
 *   [binary()], the words, broken up
 *   {'error', Reason::string()}, something went wrong
 */
ERL_NIF_TERM ubrk_words(ErlNifEnv* env, int argc,
                        const ERL_NIF_TERM argv[]) {
    ErlNifBinary str;
    ERL_NIF_TERM l;
    UBreakIterator* iter;
    int brk, lastbrk = 0;
    UErrorCode ec = U_ZERO_ERROR;

    if(!enif_inspect_binary(env, argv[0], &str))
        return enif_make_badarg(env);

    iter = ubrk_open(UBRK_WORD, NULL,
                     (UChar*)str.data, str.size/2, &ec);
    if(U_FAILURE(ec))
        return error_tuple(env, u_errorName(ec));

    l = enif_make_list(env, 0);
    while((brk=ubrk_next(iter)) != UBRK_DONE) {
        l = enif_make_list_cell(env,
                                enif_make_sub_binary(env, argv[0],
                                                     lastbrk*2,
                                                     (brk-lastbrk)*2),
                                l);
        lastbrk = brk;
    }

    ubrk_close(iter);
    return reverse_list(env, l);
}

/*
 * Initialization of this library.  Does nothing at the moment.
 */
static int on_load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
    return 0;
}

ERL_NIF_INIT(ubrk, nif_funcs, &on_load, NULL, NULL, NULL)
