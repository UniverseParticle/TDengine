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

#ifndef _TD_META_DEF_H_
#define _TD_META_DEF_H_

#include "metaUid.h"
#include "tkv.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SMeta {
  STableUidGenerator uidGenerator;

  STkvDb* tableDb;   // uid->table obj
  STkvDb* tbnameDb;  // tbname --> uid
  STkvDb* schemaDb;  // uid+version --> schema
  STkvDb* tagDb;     // uid --> tag
  STkvDb* tagIdx;    // TODO: need to integrate lucene or our own
  // STkvCache* metaCache; // TODO: add a global cache here
};

/* ------------------------ TEST CODE ------------------------ */
typedef enum { META_SUPER_TABLE = 0, META_CHILD_TABLE = 1, META_NORMAL_TABLE = 2 } EMetaTableT;
typedef struct SSuperTableOpts {
  tb_uid_t  uid;
  STSchema* pSchema;     // (ts timestamp, a int)
  STSchema* pTagSchema;  // (tag1 binary(10), tag2 int)
} SSuperTableOpts;

typedef struct SChildTableOpts {
  tb_uid_t suid;  // super table uid
  SKVRow   tags;  // tag value of the child table
} SChildTableOpts;

typedef struct SNormalTableOpts {
  STSchema* pSchema;
} SNormalTableOpts;

struct STableOpts {
  EMetaTableT type;
  char*       name;
  union {
    SSuperTableOpts  superOpts;
    SChildTableOpts  childOpts;
    SNormalTableOpts normalOpts;
  };
};

#ifdef __cplusplus
}
#endif

#endif /*_TD_META_DEF_H_*/