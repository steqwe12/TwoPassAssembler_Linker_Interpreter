#ifndef BITNO_HPP
#define BITNO_HPP

#include <list>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <iostream>


enum INSTR {
	HALT=0b00000000,
	INT=0b00010000,
	IRET1=0b10010001,
	IRET2=0b10010110,
	IRET3=0b10010010,
	CALL=0b00100001,
	RET=0b10010011,
	JMP=0b00111000,
	BEQ=0b00111001,
	BNE=0b00111010,
	BGT=0b00111011,
	PUSH=0b10000001,
	POP=0b10010011,
	XCHG=0b01000000,
	ADD=0b01010000,
	SUB=0b01010001,
	MUL=0b01010010,
	DIV=0b01010011,
	NOT=0b01100000,
	AND=0b01100001,
	OR=0b01100010,
	XOR=0b01100011,
	SHL=0b01110000,
	SHR=0b01110001,
	LD=0b10010000, // 1001xxxx ;2 ld se prevode u 2 masinske
	ST=0b10000000, // 1000xxxx
	CSRRD=0b10010000,
	CSRWR=0b10010100,
};



extern int tabSimbNumeracija;
extern unsigned int locationCounter;
extern unsigned int poolCounter;
extern char* curSect;




class TabSimb{
	public:
	int num; // simboli pocinju od 1
	unsigned int value;
	int type; // 1-NOTYP 2-SCTN
	int bind; // 1-LOC 2-GLOB
	int ndx; // 0-UND, ostalo normalno 1,2,3,..
	char* name;
	
	bool globalDef; // za .global provera
	bool externSymb; // MOOOZDA,????IDK
};

class RelZap{
	public:
	char* sekcija; // .rela.text : ali radim samo .text a ispisujem .rela.text
	
	unsigned int offset;
	int type; // msm da mi treba samo R_32_32=1
	char* simbol; // vrednost name iz TabSimb
	int addend;
};

class SadrSekc{
	public:
	char* sekcija; // .text
	
	unsigned int adr;
	unsigned char sadrzaj; 
};

class SectionDefinition{
	public:
	char *name;
	unsigned int base;
	unsigned int length;
	unsigned int poolLength;
};

class TabelaLiter{ // realno nebitno, samo jos jedna optimizacija
	public:
	char* sekcija; // .text
	
	unsigned int vrednost;
	// velicina :4B
	unsigned int lokacija; // adresa u bazenu literala
};

extern std::list<TabSimb*> TABELA_SIMBOLA;
extern std::list<RelZap*> RELOKACIONI_ZAPISI;
extern std::list<SadrSekc*> SADRZAJ_SEKCIJA;
extern std::list<SectionDefinition*> SEKCIJE;
//extern std::list<TabelaLiter*> TABELE_LITERALA;
extern std::list<char*> POMOCNI_NIZOVI_CHAROVA; // na kraju nakon ispisa i obj fajla DEALOCIRAMO SVE

//******************************************************
bool isDecimal(const char* str);

bool isHexadecimal(const char* str);

// *******************************************************************************

void dodUTabSimb_SIMBOL(char* name,unsigned int locationCounter);

void numerisiIndxSIMBOL(char* name);

void dodUTabSimb_SEKCIJA(char* name,unsigned int locationCounter);

void ubaciSadrzajUSekc(char* lit_ili_simb,unsigned int place);

void ubaciSadrzajUSekc_BYTE(unsigned char sadrzaj_za_upis,unsigned int place);

void upisiInstrukciju(unsigned char kod,char* r1,char* r2,char* r3,short pomeraj,unsigned int place);

SectionDefinition* pronadjiSekciju(char* curSect);


#endif