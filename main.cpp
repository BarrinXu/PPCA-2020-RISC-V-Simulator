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
int reg[32];
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
class IF
{

public:
    ui IR,NPC;
    void work()
    {
        IR=read_mem(PC,4);
        if(IR==0x0ff00513)
        {
            printf("%d",(ui)reg[10]&255u);
            /*cout<<"-------"<<endl;
            sort(deb.begin(),deb.end());
            for(int i=0; i<deb.size(); i++)
                if(i==0||deb[i]!=deb[i-1])
                    cout<<(bitset<7>)deb[i]<<endl;*/
            exit(0);
        }
        PC+=4;//!!!
        NPC=PC;
    }
} ;
class ID
{
public:
    ui IR,NPC;
    int A,B,Imm;
    void work(IF &if_)
    {
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
        }
        else if(opt==0b1100011)
        {
            Imm=(cal(if_.IR,31,31)<<12)|(cal(if_.IR,30,25)<<5)|(cal(if_.IR,11,8)<<1)|(cal(if_.IR,7,7)<<11);
            Imm=extend(Imm,13);
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
    void work(ID &id)
    {
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
    int work(EX &ex)
    {
        ///pc<-npc???
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
        if(ex.Cond)
            PC=ex.ALUOutput;
        return 1;
    }
};
class WB
{
public:

    void work(MEM &mem)
    {
        ui opt=mem.IR&127u;
        if(opt==0b1100011||opt==0b0100011)
            return;
        int rd=cal(mem.IR,11,7);
        if(opt==0b0000011)
            reg[rd]=mem.LMD;
        else if(opt==0b1101111||opt==0b1100111)
            reg[rd]=mem.NPC;
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
IF if_;
ID id;
EX ex;
MEM mem;
WB wb;
void solve()
{
    while(1)
    {
        if_.work();
        id.work(if_);
        ex.work(id);
        mem.work(ex);
        wb.work(mem);
    }
}
int main()
{
    pre();
    solve();
    return 0;
}
