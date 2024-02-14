#include "../inc/bitno.hpp"


int tabSimbNumeracija=0;
unsigned int locationCounter=0;
unsigned int poolCounter=0;
char* curSect=nullptr;

std::list<TabSimb*> TABELA_SIMBOLA=std::list<TabSimb*>();
std::list<RelZap*> RELOKACIONI_ZAPISI=std::list<RelZap*>();
std::list<SadrSekc*> SADRZAJ_SEKCIJA=std::list<SadrSekc*>();
std::list<SectionDefinition*> SEKCIJE=std::list<SectionDefinition*>();
//std::list<TabelaLiter*> TABELE_LITERALA;
std::list<char*> POMOCNI_NIZOVI_CHAROVA=std::list<char*>(); 

bool isDecimal(const char* str) {
    // Provjeri je li string dec broj
    for (int i = 0; str[i] != '\0'; ++i) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

bool isHexadecimal(const char* str) {
    // Provjeri je li string heks broj
    if (strlen(str) > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        for (int i = 2; str[i] != '\0'; ++i) {
            if (!isxdigit(str[i])) {
                return false;
            }
        }
        return true;
    }
    return false;
}

// *******************************************************************************

void dodUTabSimb_SIMBOL(char* name,unsigned int locationCounter){ 
	//provera da li vec postoji
	bool pronadjen = false;
	for(TabSimb* simbol : TABELA_SIMBOLA){										
			if (strcmp(simbol->name,name)==0) {
				pronadjen=true;
				if((simbol->ndx)>0) {										
					std::cerr << "multiple simbol def!" << std::endl;
					std::exit(EXIT_FAILURE);
					}
				if((simbol->globalDef)==true && (simbol->externSymb)==true){
					std::cerr << "simbol .global i .extern !!!" << std::endl;
					std::exit(EXIT_FAILURE);
				}
				if ((simbol->globalDef)==true && (simbol->externSymb)==false){
					simbol->value=locationCounter;
					simbol->type=1;
					simbol->bind=2;
					for(TabSimb* sekc : TABELA_SIMBOLA){
						if (strcmp(sekc->name,curSect)==0)		// sekc->name == curSect poredi pokazivace
							simbol->ndx=sekc->num;
					}
				}
				if ((simbol->globalDef)==false && (simbol->externSymb)==true){
					std::cerr << "simbol def i .extern !!!" << std::endl;
					std::exit(EXIT_FAILURE);
				}
				if (simbol->type==2){
					std::cerr << "simbol def i .section def !!!" << std::endl;
					std::exit(EXIT_FAILURE);
				}
				
			}
		}

	if (!pronadjen){  		// ako nije pronadjen ubacujemo u tabelu simbola

	TabSimb* simbol = new TabSimb();
	//simbol.num=++tabSimbNumeracija;  U DRUGOM PROLAZU
	simbol->value=locationCounter;
	simbol->type=1;
	simbol->bind=1; 
	for(TabSimb* sekc : TABELA_SIMBOLA){
			if (strcmp(sekc->name,curSect)==0)		// sekc->name == curSect poredi pokazivace
				simbol->ndx=sekc->num;
		}
		
	simbol->name=name;
	
	//simbol.globalDef             NZM JEL TREBA JOS NESTO
	//simbol.externSymb
	
	TABELA_SIMBOLA.push_back(simbol);
	
	}
}

void numerisiIndxSIMBOL(char* name){
	for (TabSimb* simbol : TABELA_SIMBOLA) {
        if (strcmp(simbol->name,name)==0) {
			simbol->num=++tabSimbNumeracija;
		}
    }

}

void dodUTabSimb_SEKCIJA(char* name,unsigned int locationCounter){ 
	//provera da li vec postoji	
	for(TabSimb* simbol : TABELA_SIMBOLA){										
			if (strcmp(simbol->name,name)==0) {
				std::cerr << "pokusaj .section ime, ali ime vec postoji!!!" << std::endl;
				std::exit(EXIT_FAILURE);					
			}
		}
		 
	// ako ne postoji/sve je u redu
	TabSimb *simbol = new TabSimb();
	simbol->num=++tabSimbNumeracija;
	simbol->value=locationCounter; // treba da bude 0 svejedno
	simbol->type=2;
	simbol->bind=1;
	simbol->ndx=simbol->num;
	simbol->name=name;
	
	//simbol.globalDef             NZM JEL TREBA JOS NESTO
	//simbol.externSymb
	
	TABELA_SIMBOLA.push_back(simbol);
	
	
}

void ubaciSadrzajUSekc(char* lit_ili_simb,unsigned int place){ //        4B se ubacuje
	// LITERAL
	if (isDecimal(lit_ili_simb) || isHexadecimal(lit_ili_simb)){  
		unsigned int broj = std::strtoul(lit_ili_simb, nullptr, 0);
		bool postoji=false;
		
		for(int i=place;i<place+4;i++){ // 4 adrese pocevsi od place
					
			for(SadrSekc* sadrzaj : SADRZAJ_SEKCIJA){ // pronaci da li postoji adresa sa sadrzajem u SADRZAJ_SEKCIJA
				if(strcmp(sadrzaj->sekcija,curSect)==0 && sadrzaj->adr==i){
					postoji=true;
					sadrzaj->sadrzaj=static_cast<unsigned char>(broj&0xFF);
				}
			}
			if(!postoji){
				SadrSekc* ss=new SadrSekc();
				ss->sekcija=curSect;
				ss->adr=i;
				ss->sadrzaj=static_cast<unsigned char>(broj&0xFF);
				SADRZAJ_SEKCIJA.push_back(ss);
			}
			postoji=false; // reset za pretragu da li vec postoji adresa sa sadrzajem u SADRZAJ_SEKCIJA
			broj=broj>>8;
		}
		
	}
	// SIMBOL 
	else {	          //relokacioni zapis mora da se napravi
		bool helpbool=false;
		for(TabSimb* tabsi : TABELA_SIMBOLA){	// ako korisceni simbol nije u tabeli simbola
			if(strcmp(lit_ili_simb,tabsi->name)==0) helpbool=true;
		}
		if(helpbool==false){
			std::cerr << "korisceni simbol nije u tabeli simbola" << std::endl;
		std::exit(EXIT_FAILURE);
		}

		bool postoji=false;
		
		for(int i=place;i<place+4;i++){ // 4 adrese pocevsi od place
					
			for(SadrSekc* sadrzaj : SADRZAJ_SEKCIJA){ // pronaci da li postoji adresa sa sadrzajem u SADRZAJ_SEKCIJA
				if(strcmp(sadrzaj->sekcija,curSect)==0 && sadrzaj->adr==i){
					postoji=true;
					sadrzaj->sadrzaj=0x00;
				}
			}
			if(!postoji){
				SadrSekc* ss=new SadrSekc();
				ss->sekcija=curSect;
				ss->adr=i;
				ss->sadrzaj=0x00;
				SADRZAJ_SEKCIJA.push_back(ss);
			}
			postoji=false; // reset za pretragu da li vec postoji adresa sa sadrzajem u SADRZAJ_SEKCIJA
		}
		
		// ovde za relokaciju
		RelZap* relzap=new RelZap();
		relzap->sekcija=curSect;
		relzap->offset=place;
		relzap->type=1;
		for(TabSimb* simbol : TABELA_SIMBOLA){
			if(strcmp(simbol->name,lit_ili_simb)==0){
				if(simbol->bind==2){ // simbol je globalan
					relzap->addend=0;
					relzap->simbol=lit_ili_simb;
				}
				else{  //bind==1	 // simbol je lokalan
					relzap->addend=static_cast<int>(simbol->value);
					
					for(TabSimb* trazSekc : TABELA_SIMBOLA){
						if(simbol->ndx==trazSekc->num) relzap->simbol=trazSekc->name;
					}
					
				}
				
			}
		}
		RELOKACIONI_ZAPISI.push_back(relzap);
	}
}

void ubaciSadrzajUSekc_BYTE(unsigned char sadrzaj_za_upis,unsigned int place){		// 1B 
	bool postoji=false;
	for(SadrSekc* sadrzaj : SADRZAJ_SEKCIJA){ // pronaci da li postoji adresa sa sadrzajem u SADRZAJ_SEKCIJA
		if(strcmp(sadrzaj->sekcija,curSect)==0 && sadrzaj->adr==place){
			postoji=true;
			sadrzaj->sadrzaj=sadrzaj_za_upis;
		}
	}
	if(!postoji){
		SadrSekc* ss=new SadrSekc();
		ss->sekcija=curSect;
		ss->adr=place;
		ss->sadrzaj=sadrzaj_za_upis;
		SADRZAJ_SEKCIJA.push_back(ss);
	}
}

//static_cast<unsigned char>(broj&0xFF); MOZDA AKO ZATREBA
void upisiInstrukciju(unsigned char kod,char* r1,char* r2,char* r3,short pomeraj,unsigned int place){ // forma argumenata %r1 npr
	if (pomeraj>2047 || pomeraj<-2048){
		std::cerr << "pomeraj veci od 12 bita" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	unsigned char prvi,drugi=0,treci=0,cetvrti=0;
	prvi=kod;
	if(strcmp(r1,"%status")==0 || strcmp(r1,"%r0")==0) drugi=drugi|0x00;
	else if(strcmp(r1,"%handler")==0 || strcmp(r1,"%r1")==0) drugi=drugi|0x10;
	else if(strcmp(r1,"%cause")==0 || strcmp(r1,"%r2")==0) drugi=drugi|0x20;
	else if(strcmp(r1,"%r3")==0) drugi=drugi|0x30;
	else if(strcmp(r1,"%r4")==0) drugi=drugi|0x40;
	else if(strcmp(r1,"%r5")==0) drugi=drugi|0x50;
	else if(strcmp(r1,"%r6")==0) drugi=drugi|0x60;
	else if(strcmp(r1,"%r7")==0) drugi=drugi|0x70;
	else if(strcmp(r1,"%r8")==0) drugi=drugi|0x80;
	else if(strcmp(r1,"%r9")==0) drugi=drugi|0x90;
	else if(strcmp(r1,"%r10")==0) drugi=drugi|0xA0;
	else if(strcmp(r1,"%r11")==0) drugi=drugi|0xB0;
	else if(strcmp(r1,"%r12")==0) drugi=drugi|0xC0;
	else if(strcmp(r1,"%r13")==0) drugi=drugi|0xD0;
	else if(strcmp(r1,"%r14")==0 || strcmp(r1,"%sp")==0) drugi=drugi|0xE0;
	else if(strcmp(r1,"%r15")==0 || strcmp(r1,"%pc")==0) drugi=drugi|0xF0;
	
	if(strcmp(r2,"%status")==0 || strcmp(r2,"%r0")==0) drugi=drugi|0x00;
	else if(strcmp(r2,"%handler")==0 || strcmp(r2,"%r1")==0) drugi=drugi|0x01;
	else if(strcmp(r2,"%cause")==0 || strcmp(r2,"%r2")==0) drugi=drugi|0x02;
	else if(strcmp(r2,"%r3")==0) drugi=drugi|0x03;
	else if(strcmp(r2,"%r4")==0) drugi=drugi|0x04;
	else if(strcmp(r2,"%r5")==0) drugi=drugi|0x05;
	else if(strcmp(r2,"%r6")==0) drugi=drugi|0x06;
	else if(strcmp(r2,"%r7")==0) drugi=drugi|0x07;
	else if(strcmp(r2,"%r8")==0) drugi=drugi|0x08;
	else if(strcmp(r2,"%r9")==0) drugi=drugi|0x09;
	else if(strcmp(r2,"%r10")==0) drugi=drugi|0x0A;
	else if(strcmp(r2,"%r11")==0) drugi=drugi|0x0B;
	else if(strcmp(r2,"%r12")==0) drugi=drugi|0x0C;
	else if(strcmp(r2,"%r13")==0) drugi=drugi|0x0D;
	else if(strcmp(r2,"%r14")==0 || strcmp(r2,"%sp")==0) drugi=drugi|0x0E;
	else if(strcmp(r2,"%r15")==0 || strcmp(r2,"%pc")==0) drugi=drugi|0x0F;
	
	if(strcmp(r3,"%status")==0 || strcmp(r3,"%r0")==0) treci=treci|0x00;
	else if(strcmp(r3,"%handler")==0 || strcmp(r3,"%r1")==0) treci=treci|0x10;
	else if(strcmp(r3,"%cause")==0 || strcmp(r3,"%r2")==0) treci=treci|0x20;
	else if(strcmp(r3,"%r3")==0) treci=treci|0x30;
	else if(strcmp(r3,"%r4")==0) treci=treci|0x40;
	else if(strcmp(r3,"%r5")==0) treci=treci|0x50;
	else if(strcmp(r3,"%r6")==0) treci=treci|0x60;
	else if(strcmp(r3,"%r7")==0) treci=treci|0x70;
	else if(strcmp(r3,"%r8")==0) treci=treci|0x80;
	else if(strcmp(r3,"%r9")==0) treci=treci|0x90;
	else if(strcmp(r3,"%r10")==0) treci=treci|0xA0;
	else if(strcmp(r3,"%r11")==0) treci=treci|0xB0;
	else if(strcmp(r3,"%r12")==0) treci=treci|0xC0;
	else if(strcmp(r3,"%r13")==0) treci=treci|0xD0;
	else if(strcmp(r3,"%r14")==0 || strcmp(r3,"%sp")==0) treci=treci|0xE0;
	else if(strcmp(r3,"%r15")==0 || strcmp(r3,"%pc")==0) treci=treci|0xF0;
	
	
	cetvrti=static_cast<unsigned char>(pomeraj&0x00FF);
	pomeraj=pomeraj>>8;
	pomeraj=pomeraj&0x000F;
	treci=treci|pomeraj;
	
	bool postoji=false;
					
	for(SadrSekc* sadrzaj : SADRZAJ_SEKCIJA){ // pronaci da li postoji adresa sa sadrzajem u SADRZAJ_SEKCIJA
		if(strcmp(sadrzaj->sekcija,curSect)==0 && sadrzaj->adr==place){
			postoji=true;
			sadrzaj->sadrzaj=static_cast<unsigned char>(prvi);
		}
	}
	if(!postoji){
		SadrSekc* ss=new SadrSekc();
		ss->sekcija=curSect;
		ss->adr=place;
		ss->sadrzaj=static_cast<unsigned char>(prvi);
		SADRZAJ_SEKCIJA.push_back(ss);
	}
	postoji=false;
	////////////////
	for(SadrSekc* sadrzaj : SADRZAJ_SEKCIJA){ // pronaci da li postoji adresa sa sadrzajem u SADRZAJ_SEKCIJA
		if(strcmp(sadrzaj->sekcija,curSect)==0 && sadrzaj->adr==(place+1)){
			postoji=true;
			sadrzaj->sadrzaj=static_cast<unsigned char>(drugi);
		}
	}
	if(!postoji){
		SadrSekc* ss=new SadrSekc();
		ss->sekcija=curSect;
		ss->adr=place+1;
		ss->sadrzaj=static_cast<unsigned char>(drugi);
		SADRZAJ_SEKCIJA.push_back(ss);
	}
	postoji=false;
	//////////////////////
	for(SadrSekc* sadrzaj : SADRZAJ_SEKCIJA){ // pronaci da li postoji adresa sa sadrzajem u SADRZAJ_SEKCIJA
		if(strcmp(sadrzaj->sekcija,curSect)==0 && sadrzaj->adr==(place+2)){
			postoji=true;
			sadrzaj->sadrzaj=static_cast<unsigned char>(treci);
		}
	}
	if(!postoji){
		SadrSekc* ss=new SadrSekc();
		ss->sekcija=curSect;
		ss->adr=place+2;
		ss->sadrzaj=static_cast<unsigned char>(treci);
		SADRZAJ_SEKCIJA.push_back(ss);
	}
	postoji=false;
	////////////////
	for(SadrSekc* sadrzaj : SADRZAJ_SEKCIJA){ // pronaci da li postoji adresa sa sadrzajem u SADRZAJ_SEKCIJA
		if(strcmp(sadrzaj->sekcija,curSect)==0 && sadrzaj->adr==(place+3)){
			postoji=true;
			sadrzaj->sadrzaj=static_cast<unsigned char>(cetvrti);
		}
	}
	if(!postoji){
		SadrSekc* ss=new SadrSekc();
		ss->sekcija=curSect;
		ss->adr=place+3;
		ss->sadrzaj=static_cast<unsigned char>(cetvrti);
		SADRZAJ_SEKCIJA.push_back(ss);
	}
	
}

SectionDefinition* pronadjiSekciju(char* curSect){
	for (SectionDefinition* sekcija : SEKCIJE){
		if(strcmp(sekcija->name,curSect)==0){
			return sekcija;
		}
	}
  return nullptr; // malo unsafe ali ne moze da se desi realno
}