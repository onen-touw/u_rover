#pragma once

#include "u_drivers/uart"

#define JET_ADR "1"
#define JET_RAC "RAC"
#define JET_RS1 "RS1"
#define JET_RFI "RFI"
#define JET_DUMMY "1"


void JET_GenJetCmd(char* buf, const char* cmd){
    buf[2]= cmd[0];
    buf[3]= cmd[1];
    buf[4]= cmd[2];
}

void jet(drv::UFO_Uart* drv){
    char cmd[8]={};
    cmd[0]='1';
    cmd[1]=',';
    cmd[2]='R';
    cmd[3]='A';
    cmd[4]='C';
    cmd[5]=',';
    cmd[6]='1';
    cmd[7]='\r';

    JET_GenJetCmd(cmd, JET_RAC);
    drv->SendMsg(cmd, 8);
}