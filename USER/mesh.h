//mesh.h
#ifndef _MESH_H_
#define _MESH_H_

#include "radiolink.h"

#define NODES_CNT 16
#define MMIO16(addr)  (*(volatile uint16_t *)(addr))
#define MMIO32(addr)  (*(volatile uint32_t *)(addr))
#define U_ID          0x1ffff7e8

extern struct u_id HostID;

extern uint8_t HostIDArr[12];//HOST ID
extern uint8_t Slave1IDArr[12];// S1 ID

extern uint8_t data_buff[27];
extern bool IsMaster;
extern bool RemoteConnected;
extern bool MasterSync;

/* Returns true if IDs are the same */
bool uid_cmp(struct u_id *id1, struct u_id *id2);
/* Read U_ID register */
void uid_read(struct u_id *id);

bool MeshInit();

typedef struct 
{
	uint8_t senderId;
	uint8_t nodeId;
	uint8_t data;
	uint32_t timestamp;
} meshPacket;

extern meshPacket mData[16];

#endif //_MESH_H_