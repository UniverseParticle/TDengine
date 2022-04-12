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

#include "syncRaftLog.h"
#include "wal.h"

SSyncLogStore* logStoreCreate(SSyncNode* pSyncNode) {
  SSyncLogStore* pLogStore = taosMemoryMalloc(sizeof(SSyncLogStore));
  assert(pLogStore != NULL);

  pLogStore->data = taosMemoryMalloc(sizeof(SSyncLogStoreData));
  assert(pLogStore->data != NULL);

  SSyncLogStoreData* pData = pLogStore->data;
  pData->pSyncNode = pSyncNode;
  pData->pWal = pSyncNode->pWal;

  pLogStore->appendEntry = logStoreAppendEntry;
  pLogStore->getEntry = logStoreGetEntry;
  pLogStore->truncate = logStoreTruncate;
  pLogStore->getLastIndex = logStoreLastIndex;
  pLogStore->getLastTerm = logStoreLastTerm;
  pLogStore->updateCommitIndex = logStoreUpdateCommitIndex;
  pLogStore->getCommitIndex = logStoreGetCommitIndex;
  return pLogStore;
}

void logStoreDestory(SSyncLogStore* pLogStore) {
  if (pLogStore != NULL) {
    taosMemoryFree(pLogStore->data);
    taosMemoryFree(pLogStore);
  }
}

int32_t logStoreAppendEntry(SSyncLogStore* pLogStore, SSyncEntry* pEntry) {
  SSyncLogStoreData* pData = pLogStore->data;
  SWal*              pWal = pData->pWal;

  SyncIndex lastIndex = logStoreLastIndex(pLogStore);
  assert(pEntry->index == lastIndex + 1);
  uint32_t len;
  char*    serialized = syncEntrySerialize(pEntry, &len);
  assert(serialized != NULL);

  int code = 0;
  /*
    code = walWrite(pWal, pEntry->index, pEntry->entryType, serialized, len);
    assert(code == 0);
  */
  assert(walWrite(pWal, pEntry->index, pEntry->entryType, serialized, len) == 0);

  walFsync(pWal, true);
  taosMemoryFree(serialized);
  return code;
}

SSyncEntry* logStoreGetEntry(SSyncLogStore* pLogStore, SyncIndex index) {
  SSyncLogStoreData* pData = pLogStore->data;
  SWal*              pWal = pData->pWal;
  SSyncEntry*        pEntry = NULL;

  if (index >= SYNC_INDEX_BEGIN && index <= logStoreLastIndex(pLogStore)) {
    SWalReadHandle* pWalHandle = walOpenReadHandle(pWal);
    assert(walReadWithHandle(pWalHandle, index) == 0);
    pEntry = syncEntryDeserialize(pWalHandle->pHead->head.body, pWalHandle->pHead->head.len);
    assert(pEntry != NULL);

    // need to hold, do not new every time!!
    walCloseReadHandle(pWalHandle);
  }

  return pEntry;
}

int32_t logStoreTruncate(SSyncLogStore* pLogStore, SyncIndex fromIndex) {
  SSyncLogStoreData* pData = pLogStore->data;
  SWal*              pWal = pData->pWal;
  assert(walRollback(pWal, fromIndex) == 0);
  return 0;  // to avoid compiler error
}

SyncIndex logStoreLastIndex(SSyncLogStore* pLogStore) {
  SSyncLogStoreData* pData = pLogStore->data;
  SWal*              pWal = pData->pWal;
  SyncIndex          lastIndex = walGetLastVer(pWal);
  return lastIndex;
}

SyncTerm logStoreLastTerm(SSyncLogStore* pLogStore) {
  SyncTerm    lastTerm = 0;
  SSyncEntry* pLastEntry = logStoreGetLastEntry(pLogStore);
  if (pLastEntry != NULL) {
    lastTerm = pLastEntry->term;
    taosMemoryFree(pLastEntry);
  }
  return lastTerm;
}

int32_t logStoreUpdateCommitIndex(SSyncLogStore* pLogStore, SyncIndex index) {
  SSyncLogStoreData* pData = pLogStore->data;
  SWal*              pWal = pData->pWal;
  assert(walCommit(pWal, index) == 0);
  return 0;  // to avoid compiler error
}

SyncIndex logStoreGetCommitIndex(SSyncLogStore* pLogStore) {
  SSyncLogStoreData* pData = pLogStore->data;
  return pData->pSyncNode->commitIndex;
}

SSyncEntry* logStoreGetLastEntry(SSyncLogStore* pLogStore) {
  SSyncLogStoreData* pData = pLogStore->data;
  SWal*              pWal = pData->pWal;
  SyncIndex          lastIndex = walGetLastVer(pWal);

  SSyncEntry* pEntry = NULL;
  if (lastIndex > 0) {
    pEntry = logStoreGetEntry(pLogStore, lastIndex);
  }
  return pEntry;
}

cJSON* logStore2Json(SSyncLogStore* pLogStore) {
  char               u64buf[128];
  SSyncLogStoreData* pData = (SSyncLogStoreData*)pLogStore->data;
  cJSON*             pRoot = cJSON_CreateObject();

  if (pData != NULL && pData->pWal != NULL) {
    snprintf(u64buf, sizeof(u64buf), "%p", pData->pSyncNode);
    cJSON_AddStringToObject(pRoot, "pSyncNode", u64buf);
    snprintf(u64buf, sizeof(u64buf), "%p", pData->pWal);
    cJSON_AddStringToObject(pRoot, "pWal", u64buf);
    snprintf(u64buf, sizeof(u64buf), "%ld", logStoreLastIndex(pLogStore));
    cJSON_AddStringToObject(pRoot, "LastIndex", u64buf);
    snprintf(u64buf, sizeof(u64buf), "%lu", logStoreLastTerm(pLogStore));
    cJSON_AddStringToObject(pRoot, "LastTerm", u64buf);

    cJSON* pEntries = cJSON_CreateArray();
    cJSON_AddItemToObject(pRoot, "pEntries", pEntries);
    SyncIndex lastIndex = logStoreLastIndex(pLogStore);
    for (SyncIndex i = 0; i <= lastIndex; ++i) {
      SSyncEntry* pEntry = logStoreGetEntry(pLogStore, i);
      cJSON_AddItemToArray(pEntries, syncEntry2Json(pEntry));
      syncEntryDestory(pEntry);
    }
  }

  cJSON* pJson = cJSON_CreateObject();
  cJSON_AddItemToObject(pJson, "SSyncLogStore", pRoot);
  return pJson;
}

char* logStore2Str(SSyncLogStore* pLogStore) {
  cJSON* pJson = logStore2Json(pLogStore);
  char*  serialized = cJSON_Print(pJson);
  cJSON_Delete(pJson);
  return serialized;
}

cJSON* logStoreSimple2Json(SSyncLogStore* pLogStore) {
  char               u64buf[128];
  SSyncLogStoreData* pData = (SSyncLogStoreData*)pLogStore->data;
  cJSON*             pRoot = cJSON_CreateObject();

  if (pData != NULL && pData->pWal != NULL) {
    snprintf(u64buf, sizeof(u64buf), "%p", pData->pSyncNode);
    cJSON_AddStringToObject(pRoot, "pSyncNode", u64buf);
    snprintf(u64buf, sizeof(u64buf), "%p", pData->pWal);
    cJSON_AddStringToObject(pRoot, "pWal", u64buf);
    snprintf(u64buf, sizeof(u64buf), "%ld", logStoreLastIndex(pLogStore));
    cJSON_AddStringToObject(pRoot, "LastIndex", u64buf);
    snprintf(u64buf, sizeof(u64buf), "%lu", logStoreLastTerm(pLogStore));
    cJSON_AddStringToObject(pRoot, "LastTerm", u64buf);
  }

  cJSON* pJson = cJSON_CreateObject();
  cJSON_AddItemToObject(pJson, "SSyncLogStoreSimple", pRoot);
  return pJson;
}

char* logStoreSimple2Str(SSyncLogStore* pLogStore) {
  cJSON* pJson = logStoreSimple2Json(pLogStore);
  char*  serialized = cJSON_Print(pJson);
  cJSON_Delete(pJson);
  return serialized;
}

// for debug -----------------
void logStorePrint(SSyncLogStore* pLogStore) {
  char* serialized = logStore2Str(pLogStore);
  printf("logStorePrint | len:%lu | %s \n", strlen(serialized), serialized);
  fflush(NULL);
  taosMemoryFree(serialized);
}

void logStorePrint2(char* s, SSyncLogStore* pLogStore) {
  char* serialized = logStore2Str(pLogStore);
  printf("logStorePrint | len:%lu | %s | %s \n", strlen(serialized), s, serialized);
  fflush(NULL);
  taosMemoryFree(serialized);
}

void logStoreLog(SSyncLogStore* pLogStore) {
  char* serialized = logStore2Str(pLogStore);
  sTrace("logStorePrint | len:%lu | %s", strlen(serialized), serialized);
  taosMemoryFree(serialized);
}

void logStoreLog2(char* s, SSyncLogStore* pLogStore) {
  char* serialized = logStore2Str(pLogStore);
  sTrace("logStorePrint | len:%lu | %s | %s", strlen(serialized), s, serialized);
  taosMemoryFree(serialized);
}

// for debug -----------------
void logStoreSimplePrint(SSyncLogStore* pLogStore) {
  char* serialized = logStoreSimple2Str(pLogStore);
  printf("logStoreSimplePrint | len:%lu | %s \n", strlen(serialized), serialized);
  fflush(NULL);
  taosMemoryFree(serialized);
}

void logStoreSimplePrint2(char* s, SSyncLogStore* pLogStore) {
  char* serialized = logStoreSimple2Str(pLogStore);
  printf("logStoreSimplePrint2 | len:%lu | %s | %s \n", strlen(serialized), s, serialized);
  fflush(NULL);
  taosMemoryFree(serialized);
}

void logStoreSimpleLog(SSyncLogStore* pLogStore) {
  char* serialized = logStoreSimple2Str(pLogStore);
  sTrace("logStoreSimpleLog | len:%lu | %s", strlen(serialized), serialized);
  taosMemoryFree(serialized);
}

void logStoreSimpleLog2(char* s, SSyncLogStore* pLogStore) {
  char* serialized = logStoreSimple2Str(pLogStore);
  sTrace("logStoreSimpleLog2 | len:%lu | %s | %s", strlen(serialized), s, serialized);
  taosMemoryFree(serialized);
}

// ======================

SSyncRaftLog* syncRaftLogCreate(SSyncNode* pSyncNode) {
  SSyncRaftLog* pLog = taosMemoryMalloc(sizeof(SSyncRaftLog));
  assert(pLog != NULL);

  pLog->data = taosMemoryMalloc(sizeof(SSyncRaftLogData));
  assert(pLog->data != NULL);

  SSyncRaftLogData* pData = pLog->data;
  pData->pSyncNode = pSyncNode;
  pData->pWal = pSyncNode->pWal;

  pLog->appendEntry = syncRaftLogAppendEntry;
  pLog->getEntry = syncRaftLogGetEntry;
  pLog->truncate = syncRaftLogTruncate;
  pLog->getLastIndex = syncRaftLogLastIndex;
  pLog->getLastTerm = syncRaftLogLastTerm;
  pLog->updateCommitIndex = syncRaftLogUpdateCommitIndex;
  pLog->getCommitIndex = syncRaftLogGetCommitIndex;
  return pLog;
}

void syncRaftLogDestory(SSyncRaftLog* pLog) {
  if (pLog != NULL) {
    taosMemoryFree(pLog->data);
    taosMemoryFree(pLog);
  }
}

int32_t syncRaftLogAppendEntry(SSyncRaftLog* pLog, SSyncRaftEntry* pEntry) {}

SSyncRaftEntry* syncRaftLogGetEntry(SSyncRaftLog* pLog, SyncIndex index) {}

int32_t syncRaftLogTruncate(SSyncRaftLog* pLog, SyncIndex fromIndex) {}

SyncIndex syncRaftLogLastIndex(SSyncRaftLog* pLog) {}

SyncTerm syncRaftLogLastTerm(SSyncRaftLog* pLog) {}

int32_t syncRaftLogUpdateCommitIndex(SSyncRaftLog* pLog, SyncIndex index) {}

SyncIndex syncRaftLogGetCommitIndex(SSyncRaftLog* pLog) {}

SSyncRaftEntry* syncRaftLogGetLastEntry(SSyncRaftLog* pLog) {}

cJSON* syncRaftLog2Json(SSyncRaftLog* pLog) {}

char* syncRaftLog2Str(SSyncRaftLog* pLog) {}

cJSON* syncRaftLogSimple2Json(SSyncRaftLog* pLog) {}

char* syncRaftLogSimple2Str(SSyncRaftLog* pLog) {}

// for debug
void syncRaftLogPrint(SSyncRaftLog* pLog) {}

void syncRaftLogPrint2(char* s, SSyncRaftLog* pLog) {}

void syncRaftLogLog(SSyncRaftLog* pLog) {}

void syncRaftLogLog2(char* s, SSyncRaftLog* pLog) {}

void syncRaftLogSimplePrint(SSyncRaftLog* pLog) {}

void syncRaftLogSimplePrint2(char* s, SSyncRaftLog* pLog) {}

void syncRaftLogSimpleLog(SSyncRaftLog* pLog) {}

void syncRaftLogSimpleLog2(char* s, SSyncRaftLog* pLog) {}