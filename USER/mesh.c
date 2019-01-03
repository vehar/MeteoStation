#include "mesh.h"
#include "debug.h"

struct u_id HostID;

uint8_t HostIDArr[12] = {0x48, 0xFF, 0x6C, 0x06, 0x50, 0x77, 0x51, 0x49, 0x43, 0x35, 0x20, 0x87};//HOST ID
uint8_t Slave1IDArr[12] = {0x52, 0xFF, 0x70, 0x06, 0x49, 0x84, 0x55, 0x50, 0x12, 0x36, 0x11, 0x87};// S1 ID

uint8_t HostIDArr1[12] = {0x54, 0xFF, 0x68, 0x06, 0x78, 0x71, 0x57, 0x51, 0x25, 0x54, 0x13, 0x67};//HOST ID //#1
uint8_t HostIDArr2[12] = {0x54, 0xFF, 0x6E, 0x06, 0x78, 0x71, 0x57, 0x51, 0x23, 0x30, 0x13, 0x67};//HOST ID //#2

uint8_t data_buff[27];

meshPacket mData[NODES_CNT];

SensorFile_t sFile;

bool IsMaster = false;
bool RemoteConnected = false;
bool MasterSync = false;

/* Read U_ID register */
void uid_read(struct u_id *id)
{
    id->off0 = MMIO16(U_ID + 0x0);
    id->off2 = MMIO16(U_ID + 0x2);
    id->off4 = MMIO32(U_ID + 0x4);
    id->off8 = MMIO32(U_ID + 0x8);
}

/* Returns true if IDs are the same */
bool uid_cmp(struct u_id *id1, struct u_id *id2)
{
    return id1->off0 == id2->off0 &&
           id1->off2 == id2->off2 &&
           id1->off4 == id2->off4 &&
           id1->off8 == id2->off8;
}

/*
    struct u_id id1 = { 0x0, 0x1, 0x2, 0x3 };
    struct u_id id2;
    bool same_id;

    uid_read(&id2);
    same_id = uid_cmp(&id1, &id2);

    printf("%s\n", same_id ? "equal" : "not equal");
*/

bool MeshInit()
{
//	DEBUGFMSG("");
	
	struct u_id idn;	
	memset(mData, 0, sizeof(meshPacket)*NODES_CNT);
	
	uid_read(&HostID);
	memcpy(&idn, &HostIDArr2, sizeof(HostIDArr1));
	memcpy(&sFile.senderId, &HostID, sizeof(HostID));
	
  IsMaster = uid_cmp(&HostID, &idn);
	return IsMaster;
}
