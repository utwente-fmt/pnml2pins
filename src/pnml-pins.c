/*
 * Copyright 2015 Formal Methods and Tools, University of Twente
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <dlfcn.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include <ltsmin/pins.h>
#include <ltsmin/dlopen-api.h>
#include <ltsmin/lts-type.h>
#include <ltsmin/ltsmin-standard.h>

#include "pnml-pins.h"

#define cas(a, b, c) __sync_bool_compare_and_swap(a,b,c)

static uint32_t max_token_count = 0;

void pnml_exit(model_t model) {
    fprintf(stderr, "pnml: max token count: %u\n", max_token_count);
}

int pnml_get_successor(void* model, int t, int *in, void (*callback)(void* arg, transition_info_t *transition_info, int *out, int *cpy), void *arg)
{
    int out[pnml_vars];
    memcpy(out, in, sizeof(int[pnml_vars]));

    // guard this transition, works for multiple in-arcs
    const int in_arcs = pnml_sources[t][0];
    for (int i = 0; i < in_arcs; i++) {
        int* const place = &(out[pnml_sources[t][i*2+1]]);
        const int num = pnml_sources[t][i*2+1+1];
        *place -= num;
        if (*place < 0) return 0;
    }

    // add token, works for multiple out-arcs
    const int out_arcs = pnml_targets[t][0];
    for (int i = 0; i < out_arcs; i++) {
        const int pos = pnml_targets[t][i*2+1];
        int* const place = &(out[pos]);
        if (!pnml_safe_places[pos]) {
            const int num = pnml_targets[t][i*2+1+1];
            *place += num;

            // detect overflow
            if (*place < 0) {
                fprintf(stderr, "pnml: ERROR: max token count exceeded\n");
                ltsmin_abort(LTSMIN_EXIT_FAILURE);
            }
        } else *place = 1;

        volatile uint32_t* ptr;
        do {
            ptr = &max_token_count;
            if (*place <= *ptr) break;
        } while(!cas(ptr, *ptr, *place));
    }

    int transition_labels[1] = {0};
    transition_info_t transition_info = { transition_labels, t };

    callback(arg, &transition_info, out, NULL);

    return 1;
}

void pins_model_init(model_t model)
{
    lts_type_t ltstype;
    matrix_t *dm_info = malloc(sizeof(matrix_t));
    matrix_t *dm_read_info = malloc(sizeof(matrix_t));
    matrix_t *dm_must_write_info = malloc(sizeof(matrix_t));

    // get ltstypes
    ltstype=lts_type_create();
    const int state_length = pnml_vars;

    // adding types
    int ntypes = 2;
    int int_type = lts_type_add_type(ltstype, "int", NULL);
    int act_type =lts_type_add_type(ltstype, "action", NULL);

    lts_type_set_format (ltstype, int_type, LTStypeDirect);
    lts_type_set_format (ltstype, act_type, LTStypeEnum);

    lts_type_set_state_length(ltstype, state_length);

    // set state name & type
    for (int i=0; i < state_length; ++i) {
        lts_type_set_state_name(ltstype,i,pnml_var_names[i]);
        lts_type_set_state_typeno(ltstype,i,int_type);
    }

    // edge label types
    lts_type_set_edge_label_count (ltstype, 1);
    lts_type_set_edge_label_name(ltstype, 0, "action");
    lts_type_set_edge_label_type(ltstype, 0, "action");
    lts_type_set_edge_label_typeno(ltstype, 0, act_type);

    int bool_is_new, bool_type = lts_type_add_type(ltstype, LTSMIN_TYPE_BOOL, NULL);

    GBsetLTStype(model, ltstype); // must set ltstype before setting initial state
                                  // creates tables for types!

    if (bool_is_new) {
        GBchunkPutAt(model, bool_type, chunk_str(LTSMIN_VALUE_BOOL_FALSE), 0);
        GBchunkPutAt(model, bool_type, chunk_str(LTSMIN_VALUE_BOOL_TRUE ), 1);
    }

    // setting values for types
    for (int i = 0; i < pnml_groups; i++) {
        GBchunkPutAt(model, act_type, chunk_str((char*) pnml_group_names[i]), i);
    }

    // get initial state
    GBsetInitialState(model,pnml_initial_state);

    for (int i = 0; i < pnml_vars; i++) {
        if (max_token_count < pnml_initial_state[i]) {
            max_token_count = pnml_initial_state[i];
        }
    }

    // get next state
    GBsetNextStateLong(model, (next_method_grey_t) pnml_get_successor);

    lts_type_validate(ltstype); // done with ltstype

    // initialize the state read/write dependency matrices
    dm_create(dm_read_info, pnml_groups, state_length);
    dm_create(dm_must_write_info, pnml_groups, state_length);

    for (int i = 0; i < pnml_groups; i++) {
        for (int j = 0; j < pnml_sources[i][0]; j++) {
            dm_set(dm_read_info, i, pnml_sources[i][j*2+1]);
            dm_set(dm_must_write_info, i, pnml_sources[i][j*2+1]);
        }
        for (int j = 0; j < pnml_targets[i][0]; j++) {
            if (!pnml_safe_places[pnml_targets[i][j*2+1]]) {
                /* If the place is safe we don't need to mark it read-dependent. */
                dm_set(dm_read_info, i, pnml_targets[i][j*2+1]);
            }
            dm_set(dm_must_write_info, i, pnml_targets[i][j*2+1]);
        }
    }
    dm_copy(dm_read_info, dm_info);
    dm_apply_or(dm_info, dm_must_write_info);

    GBsetDMInfo(model, dm_info);
    GBsetDMInfoRead(model, dm_read_info);
    GBsetDMInfoMustWrite(model, dm_must_write_info);
    GBsetSupportsCopy(model);

    GBsetExit(model, pnml_exit);

    matrix_t *dna_info = malloc(sizeof(matrix_t));
    dm_create(dna_info, pnml_groups, pnml_groups);
    for (int i = 0; i < pnml_groups; i++) {
        for(int j = 0; j < pnml_groups; j++) {
            const int guards_i = pnml_sources[i][0];
            const int guards_j = pnml_sources[j][0];
            for (int g = 0; g < guards_i; g++) {
                const int guard_i = pnml_sources[i][g * 2 + 1];
                for (int h = 0; h < guards_j; h++) {
                    const int guard_j = pnml_sources[j][h * 2 + 1];
                    if (guard_i == guard_j) {
                        dm_set(dna_info, i, j);
                        goto next_dna;
                    }
                }
            }
            next_dna:;
        }
    }
    GBsetDoNotAccordInfo(model, dna_info);

    matrix_t *gnes_info = malloc(sizeof(matrix_t));
    dm_create(gnes_info, pnml_vars, pnml_groups);
    for(int i = 0; i < pnml_groups; i++) {
        const int targets = pnml_targets[i][0];
        for (int t = 0; t < targets; t++) {
            const int target = pnml_targets[i][t * 2 + 1];
            dm_set(gnes_info, target, i);
        }
    }
    GBsetGuardNESInfo(model, gnes_info);

    matrix_t *gnds_info = malloc(sizeof(matrix_t));
    dm_create(gnds_info, pnml_vars, pnml_groups);
    for(int i = 0; i < pnml_groups; i++) {
        const int sources = pnml_sources[i][0];
        for (int s = 0; s < sources; s++) {
            const int source = pnml_sources[i][s * 2 + 1];
            dm_set(gnds_info, source, i);
        }
    }
    GBsetGuardNDSInfo(model, gnds_info);

    matrix_t *ndb_info = malloc(sizeof(matrix_t));
    dm_create(ndb_info, pnml_groups, pnml_groups);
    for (int i = 0; i < pnml_groups; i++) {
        const int sources = pnml_sources[i][0];
        for (int j = 0; j < pnml_groups; j++) {
            const int targets = pnml_targets[j][0];
            for (int s = 0; s < sources; s++) {
                const int source = pnml_sources[i][s * 2 + 1];
                for (int t = 0; t < targets; t++) {
                    const int target = pnml_targets[j][t * 2 + 1];
                    if (source == target) {
                        dm_set(ndb_info, i, j);
                        goto next_ndb;
                    }
                }
            }
            next_ndb:;
        }
    }
    GBsetMatrix(model, LTSMIN_NOT_LEFT_ACCORDS, ndb_info, PINS_STRICT, PINS_INDEX_OTHER, PINS_INDEX_OTHER);
}

// declarations required for dlopen-api
char pins_plugin_name[]="PNML";
