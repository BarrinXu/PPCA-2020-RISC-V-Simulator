#include <algorithm>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <bitset>
typedef unsigned int ui;
using namespace std;
ui Mem[1048576];
ui PC;
int reg[32],pred[4096][4];/// pred[] size should be 4096
int ht[4096];
bool jump(int pos)
{
    ///NOT JUMP(-2,-1),JUMP(0,1)
    pos/=4;
    if(pred[pos][ht[pos]]>=0)
        return true;
    else
        return false;
}
long long aa,bb;
void update_history(int pos,int opt)
{
    pos/=4;
    ht[pos]=((ht[pos]<<1)&3)|opt;
}
void update(int pos,bool opt)
{
    bb++;
    if(opt)
        aa++;
    ///NOT JUMP(-2,-1),JUMP(0,1)
    pos/=4;
    if(pred[pos][ht[pos]]>=0)
    {
        if(opt)
        {
            if(pred[pos][ht[pos]]==0)
                pred[pos][ht[pos]]++;
        }
        else
            pred[pos][ht[pos]]--;
    }
    else
    {
        if(opt)
        {
            if(pred[pos][ht[pos]]==-1)
                pred[pos][ht[pos]]--;
        }
        else
            pred[pos][ht[pos]]++;
    }
}
ui cal(ui x,ui l,ui r)
{
    return (x>>r)&((1<<(l-r+1))-1);
}
int extend(int x,int len)
{
    if(x&(1<<(len-1)))
        x|=((1<<(32-len))-1)<<len;
    return x;
}
ui read_mem(int pos,int len)
{
    ui ret=0;
    for(int i=0; i<len; i++)
        ret|=Mem[pos+i]<<(8*i);
    return ret;
}
void write_mem(int x,int pos,int len)
{
    for(int i=0; i<len; i++)
        Mem[pos+i]=(x>>(8*i))&255u;
}
bool is_end;
class IF
{

public:
    ui IR,NPC;
    bool rest;
    void work()
    {
        rest=is_end;
        if(rest)
            return;
        IR=read_mem(PC,4);
        if(IR==0x0ff00513)
            is_end=true;
        PC+=4;//!!!
        NPC=PC;
        ///special for direct jump
        if((IR&127u)==0b1101111)
        {
            int Imm=(cal(IR,31,31)<<20)|(cal(IR,30,21)<<1)|(cal(IR,20,20)<<11)|(cal(IR,19,12)<<12);
            Imm=extend(Imm,21);
            PC=NPC-4+Imm;
        }
    }
} ;
class ID
{
public:
    ui IR,NPC;
    int A,B,Imm;
    bool rest;
    void work(IF &if_)
    {
        if(if_.rest)
        {
            rest=true;
            return;
        }
        rest=false;
        IR=if_.IR,NPC=if_.NPC;///Maybe at the end
        ui opt=if_.IR&127u;
        ui tmp=cal(if_.IR,14,12);
        //cout<<(bitset<7>)opt<<' '<<(bitset<3>)tmp<<' '<<hex<<PC-4<<endl;
        if(opt!=0b0110111&&opt!=0b0010111&&opt!=0b1101111)
        {
            A=reg[(if_.IR>>15)&31u];//A=reg[rs1]
            if(opt!=0b1100111&&opt!=0b0000011&&opt!=0b0010011)
                B=reg[(if_.IR>>20)&31u];//B=reg[rs2]
        }
        Imm=0;

        if(opt==0b0110111||opt==0b0010111)
            Imm=(if_.IR>>12)<<12;
        else if(opt==0b1101111)
        {
            Imm=(cal(if_.IR,31,31)<<20)|(cal(if_.IR,30,21)<<1)|(cal(if_.IR,20,20)<<11)|(cal(if_.IR,19,12)<<12);
            Imm=extend(Imm,21);
        }
        else if(opt==0b1100111)
        {
            Imm=cal(if_.IR,31,20);
            Imm=extend(Imm,12);

            ///special jump
            PC=(A+Imm)&(~1);

        }
        else if(opt==0b1100011)
        {
            Imm=(cal(if_.IR,31,31)<<12)|(cal(if_.IR,30,25)<<5)|(cal(if_.IR,11,8)<<1)|(cal(if_.IR,7,7)<<11);
            Imm=extend(Imm,13);

            ///special jump
            bool Cond=false;
            ui tmp=cal(if_.IR,14,12);
            if(tmp==0b000)
                Cond=(A==B);
            else if(tmp==0b001)
                Cond=(A!=B);
            else if(tmp==0b100)
                Cond=(A<B);
            else if(tmp==0b101)
                Cond=(A>=B);
            else if(tmp==0b110)
                Cond=((ui)A<(ui)B);
            else if(tmp==0b111)
                Cond=((ui)A>=(ui)B);
            if(Cond)
                PC=if_.NPC-4+Imm;

        }
        else if(opt==0b0000011)
        {
            Imm=cal(if_.IR,31,20);
            Imm=extend(Imm,12);
        }
        else if(opt==0b0100011)
        {
            Imm=(cal(if_.IR,31,25)<<5)|(cal(if_.IR,11,7));
            Imm=extend(Imm,12);
        }
        else if(opt==0b0010011)
        {
            Imm=cal(if_.IR,31,20);
            Imm=extend(Imm,12);
        }
    }
};
class EX
{
public:
    ui IR,NPC;
    int A,B,Imm;
    int ALUOutput,Cond;
    bool rest;
    void work(ID &id)
    {
        if(id.rest)
        {
            rest=true;
            return;
        }
        rest=false;
        IR=id.IR,NPC=id.NPC,A=id.A,B=id.B,Imm=id.Imm;///Maybe at the end
        Cond=0;
        ui opt=id.IR&127u;
        if(opt==0b0110111)
        {
            ALUOutput=id.Imm;
        }
        else if(opt==0b0010111)
        {
            ALUOutput=id.NPC-4+id.Imm;
        }
        else if(opt==0b1101111)
        {
            ALUOutput=id.NPC-4+id.Imm;
            Cond=1;
        }
        else if(opt==0b1100111)
        {
            ALUOutput=(id.A+id.Imm)&(~1);
            Cond=1;
        }
        else if(opt==0b1100011)
        {
            ALUOutput=id.NPC-4+id.Imm;
            ui tmp=cal(id.IR,14,12);
            if(tmp==0b000)
                Cond=(id.A==id.B);
            else if(tmp==0b001)
                Cond=(id.A!=id.B);
            else if(tmp==0b100)
                Cond=(id.A<id.B);
            else if(tmp==0b101)
                Cond=(id.A>=id.B);
            else if(tmp==0b110)
                Cond=((ui)id.A<(ui)id.B);
            else if(tmp==0b111)
                Cond=((ui)id.A>=(ui)id.B);
        }
        else if(opt==0b0000011)
        {
            ALUOutput=id.A+id.Imm;
        }
        else if(opt==0b0100011)
        {
            ALUOutput=id.A+id.Imm;
        }
        else if(opt==0b0010011)
        {
            ui tmp=cal(id.IR,14,12);
            if(tmp==0b000)
                ALUOutput=id.A+id.Imm;
            else if(tmp==0b010)
                ALUOutput=(id.A<id.Imm);
            else if(tmp==0b011)
                ALUOutput=((ui)id.A<(ui)id.Imm);
            else if(tmp==0b100)
                ALUOutput=id.A^id.Imm;
            else if(tmp==0b110)
                ALUOutput=id.A|id.Imm;
            else if(tmp==0b111)
                ALUOutput=id.A&id.Imm;
            else if(tmp==0b001)
                ALUOutput=id.A<<id.Imm;
            else if(tmp==0b101)
            {
                id.Imm&=31u;
                if(id.IR&(1<<30))
                    ALUOutput=id.A>>id.Imm;
                else
                    ALUOutput=(ui)id.A>>id.Imm;
            }
        }
        else if(opt==0b0110011)
        {
            ui tmp=cal(id.IR,14,12);
            if(tmp==0b000)
            {
                if(id.IR&(1<<30))
                {
                    ALUOutput=id.A-id.B;
                }
                else
                {
                    ALUOutput=id.A+id.B;
                }
            }
            else if(tmp==0b001)
                ALUOutput=id.A<<(id.B&31u);
            else if(tmp==0b010)
                ALUOutput=(id.A<id.B);
            else if(tmp==0b011)
                ALUOutput=((ui)id.A<(ui)id.B);
            else if(tmp==0b100)
                ALUOutput=id.A^id.B;
            else if(tmp==0b101)
            {
                ALUOutput=id.A>>(id.B&31u);
                if(id.IR&(1<<30))
                    ALUOutput=id.A>>(id.B&31u);
                else
                    ALUOutput=(ui)id.A>>(id.B&31u);
            }
            else if(tmp==0b110)
                ALUOutput=id.A|id.B;
            else if(tmp==0b111)
                ALUOutput=id.A&id.B;
        }
    }
};
class MEM
{
public:
    int LMD;
    ui IR,NPC;
    int A,B,Imm;
    int ALUOutput,Cond;
    bool rest;
    int work(EX &ex)
    {
        ///pc<-npc???
        if(ex.rest)
        {
            rest=true;
            return 1;
        }
        rest=false;
        IR=ex.IR,NPC=ex.NPC,A=ex.A,B=ex.B,Imm=ex.Imm,ALUOutput=ex.ALUOutput,Cond=ex.Cond;///Maybe at the end
        ui opt=ex.IR&127u;
        if(opt==0b0000011)
        {
            ui tmp=cal(ex.IR,14,12);
            if(tmp==0b000)
                LMD=extend(read_mem(ex.ALUOutput,1),8);
            else if(tmp==0b001)
                LMD=extend(read_mem(ex.ALUOutput,2),16);
            else if(tmp==0b010)
                LMD=read_mem(ex.ALUOutput,4);
            else if(tmp==0b100)
                LMD=read_mem(ex.ALUOutput,1);
            else if(tmp==0b101)
                LMD=read_mem(ex.ALUOutput,2);
            return 3;
        }
        else if(opt==0b0100011)
        {
            ui tmp=cal(ex.IR,14,12);
            if(tmp==0b000)
                write_mem(ex.B,ex.ALUOutput,1);
            else if(tmp==0b001)
                write_mem(ex.B,ex.ALUOutput,2);
            else if(tmp==0b010)
                write_mem(ex.B,ex.ALUOutput,4);
            return 3;
        }
        ///will be done at period 1or2
        /*if(ex.Cond)
            PC=ex.ALUOutput;*/
        return 1;
    }
};
class WB
{
public:

    void work(MEM &mem)
    {
        if(mem.rest)
            return;
        if(mem.IR==0x0ff00513)
        {
            //printf("%.1f%%\n",(double)aa*100/(double)bb);
            printf("%d",(ui)reg[10]&255u);
            exit(0);
        }
        ui opt=mem.IR&127u;
        if(opt==0b1100011||opt==0b0100011)
            return;
        int rd=cal(mem.IR,11,7);
        if(opt==0b0000011)
            reg[rd]=mem.LMD;
        else if(opt==0b1101111||opt==0b1100111)
        {
            reg[rd]=mem.NPC;
        }
        else
            reg[rd]=mem.ALUOutput;
        /*if(reg[0])
            puts("ERROR!");*/
        reg[0]=0;
    }
};
char s[10];
void pre()
{
    int pos=0;
    while(scanf("%s",&s[0])!=EOF)
    {
        if(s[0]=='@')
            sscanf(&s[1],"%x",&pos);
        else
        {
            sscanf(&s[0],"%x",&Mem[pos]);
            pos++;
        }
    }
}
IF if_[2];
ID id[2];
EX ex[2];
MEM mem[2];
WB wb[2];
bool forward_rs1,forward_rs2;
bool check_data(ID &id,EX &ex)
{
    if(id.rest||ex.rest)
        return true;
    ui id_opt=id.IR&127u,ex_opt=ex.IR&127u;
    if(id_opt==0b0110111||id_opt==0b0010111||id_opt==0b1101111)
        return true;
    if(ex_opt==0b1100011||ex_opt==0b0100011)
        return true;
    int rs1=(id.IR>>15)&31u;
    int rs2=(id.IR>>20)&31u;
    if(id_opt==0b1100111||id_opt==0b0000011||id_opt==0b0010011)
        rs2=-1;
    int rd=cal(ex.IR,11,7);
    if(rd==0)
        rd=-2;
    if(rd==rs1||rd==rs2)
    {
        if(ex_opt==0b0000011||ex_opt==0b1101111||ex_opt==0b1100111)
            return false;
        if(id_opt==0b1100111||id_opt==0b1100011)
            return false;
        if(rd==rs1)
            forward_rs1=true,id.A=ex.ALUOutput;
        if(rd==rs2)
            forward_rs2=true,id.B=ex.ALUOutput;
    }
    return true;
}
bool check_data(ID &id,MEM &mem)
{
    if(id.rest||mem.rest)
        return true;
    ui id_opt=id.IR&127u,mem_opt=mem.IR&127u;
    if(id_opt==0b0110111||id_opt==0b0010111||id_opt==0b1101111)
        return true;
    if(mem_opt==0b1100011||mem_opt==0b0100011)
        return true;
    int rs1=(id.IR>>15)&31u;
    int rs2=(id.IR>>20)&31u;
    if(id_opt==0b1100111||id_opt==0b0000011||id_opt==0b0010011)
        rs2=-1;
    int rd=cal(mem.IR,11,7);
    if(rd==0)
        rd=-2;
    if(rd==rs1||rd==rs2)
    {
        if(mem_opt==0b0000011||mem_opt==0b1101111||mem_opt==0b1100111)
            return false;
        if(id_opt==0b1100111||id_opt==0b1100011)
            return false;
        if(rd==rs1&&!forward_rs1)
            id.A=mem.ALUOutput;
        if(rd==rs2&&!forward_rs2)
            id.B=mem.ALUOutput;
    }
    return true;
}
bool JALR(ui &id)
{
    return (id&127u)==0b1100111;
}
bool IF_JUMP(ui &id)
{
    return (id&127u)==0b1100011;
}
void solve()
{
    int t=0;
    if_[t].rest=id[t].rest=ex[t].rest=mem[t].rest=true;
    int clock=0;
    while(1)
    {
        t^=1;
        clock++;
        wb[t].work(mem[t^1]);
        int sp_tim=mem[t].work(ex[t^1]);
        ex[t].work(id[t^1]);
        id[t].work(if_[t^1]);
        //cout<<hex<<PC<<endl;


        forward_rs1=forward_rs2=false;
        if(!check_data(id[t],ex[t]))///data_delay 2 periods
        {
            PC=id[t].NPC-4;
            id[t].rest=if_[t].rest=true;
        }
        else if(!check_data(id[t],mem[t]))///data_delay 1 period
        {
            PC=id[t].NPC-4;
            id[t].rest=true;
            if_[t].work();
        }
        else
        {
            if(!id[t].rest&&JALR(id[t].IR))
            {
                if_[t].rest=true;
            }
            else if(!id[t].rest&&IF_JUMP(id[t].IR))
            {
                bool tag=jump(id[t].NPC-4);
                if((tag&&PC!=id[t].NPC)||(!tag&&PC==id[t].NPC))///Right Prediction
                {
                    update(id[t].NPC-4,true);
                    update_history(id[t].NPC-4,PC!=id[t].NPC);
                    if_[t].work();
                }
                else///Wrong Prediction
                {
                    update(id[t].NPC-4,false);
                    update_history(id[t].NPC-4,PC!=id[t].NPC);
                    if_[t].rest=true;
                }
            }
            else
            {
                if_[t].work();
            }
        }



        clock+=sp_tim-1;


    }
}
int main()
{
    //freopen("in.data","r",stdin);
    pre();
    solve();

    return 0;
}
