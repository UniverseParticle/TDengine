/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TD_BUILDINS_H_
#define _TD_BUILDINS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "functionMgtInt.h"

typedef int32_t (*FTranslateFunc)(SFunctionNode* pFunc, char* pErrBuf, int32_t len);
typedef EFuncDataRequired (*FFuncDataRequired)(SFunctionNode* pFunc, STimeWindow* pTimeWindow);

typedef struct SBuiltinFuncDefinition {
  char name[FUNCTION_NAME_MAX_LENGTH];
  EFunctionType type;
  uint64_t classification;
  FTranslateFunc translateFunc;
  FFuncDataRequired dataRequiredFunc;
  FExecGetEnv getEnvFunc;
  FExecInit initFunc;
  FExecProcess processFunc;
  FScalarExecProcess sprocessFunc;
  FExecFinalize finalizeFunc;
} SBuiltinFuncDefinition;

extern const SBuiltinFuncDefinition funcMgtBuiltins[];
extern const int funcMgtBuiltinsNum;

#ifdef __cplusplus
}
#endif

#endif /*_TD_BUILDINS_H_*/