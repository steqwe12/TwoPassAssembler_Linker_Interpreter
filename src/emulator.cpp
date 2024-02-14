#include <iostream>
#include <fstream>
#include <list>
#include <memory>
#include <iomanip>

#define ILLEGAL_MACRO() \
{	\
    r[14] -= 4; \
    upisiPodatak32b(status, r[14]); \
    r[14] -= 4; \
    upisiPodatak32b(r[15], r[14]); \
    cause = 1; \
    status &= 0x7; \
    r[15] = handler; \
    continue;	\
}


class EmulMem{
	public:	
	unsigned int adr;
	unsigned char sadrzaj;
};

std::list<EmulMem*> MEMORIJA=std::list<EmulMem*>();				// emulirana memorija i registri cpu

unsigned int citajPodatak32b(unsigned int adresa);
void upisiPodatak32b(unsigned int podatak,unsigned int adresa);
unsigned char dohvatiInstr(unsigned int adresa);

int main(int argc, char* argv[]) {
  std::streambuf* cerr_buffer = std::cerr.rdbuf();
  std::ofstream cerr_file("error_emul.txt");
  std::cerr.rdbuf(cerr_file.rdbuf());
	
	
	unsigned int r[16];  // r15-pc         r14-sp			r0=0(static const)
	unsigned int status=0, handler=0, cause=0;
	//           0		 1		  2
	
  for (int i = 0; i < 16; ++i) {
      r[i] = 0;
  }

	if (argc!=2) {
		std::cerr << "Greška: naveden ulazni pa argument" << std::endl;
        return 1; // Vraćamo kod greške
	}
	
	
	std::ifstream binarniFajl(argv[1], std::ios::binary);
	
	if (binarniFajl.is_open()) {
		unsigned int brojMemByte;
		binarniFajl.read(reinterpret_cast<char*>(&brojMemByte), sizeof(brojMemByte));
		
		for(int i=0;i<brojMemByte;i++){
			EmulMem* bajt = new EmulMem();
			
			binarniFajl.read(reinterpret_cast<char*>(&bajt->adr), sizeof(bajt->adr));
			binarniFajl.read(reinterpret_cast<char*>(&bajt->sadrzaj), sizeof(bajt->sadrzaj));
			
			MEMORIJA.push_back(bajt);
		}
		
    	
        binarniFajl.close();
    } else {
        std::cerr << "Nije moguće otvoriti fajl." << std::endl;
		return 1;
    }
	
	
	
	// ovde rad
	for (EmulMem* em : MEMORIJA){
		if (em->adr>=static_cast<unsigned int>(0xFFFFFF00)){
			std::cerr << "Nije moguće emuliranje, pokusaj ucitavanja na mem mapirane registre." << std::endl;
			return 1;
		}
	}
	
	// za svaki slucaj sort
	MEMORIJA.sort([](EmulMem* a, EmulMem* b) {
        return a->adr < b->adr;
    });
	
	
	
	// postavljanje inicijalnog stanja
	r[15]=static_cast<unsigned int>(0x40000000);
	
	
	unsigned char instr1, instr2, instr3, instr4;
	unsigned char AAAA, BBBB, CCCC;
	signed short DDDD;
	
	
	// SAM RAD EMULATORA 
	while (true) {
		r[0]=0;
		instr1=dohvatiInstr(r[15]);
		instr2=dohvatiInstr(r[15]+1);
		instr3=dohvatiInstr(r[15]+2);
		instr4=dohvatiInstr(r[15]+3);
		
		AAAA=static_cast<unsigned char>((instr2>>4)&0x0F);
		BBBB=static_cast<unsigned char>(instr2&0x0F);
		CCCC=static_cast<unsigned char>((instr3>>4)&0x0F);
    unsigned short helpp=((static_cast<unsigned short>(instr3) << 8) | instr4)&0x0FFF;
    if (helpp>=0x0800) DDDD = static_cast<signed short>(helpp | 0xF000);
    else DDDD = static_cast<signed short>(helpp);
		//DDDD=static_cast<signed short>(((static_cast<unsigned short>(instr3) << 8) | instr4)&0x0FFF);
		
		r[15]=r[15]+4;		// pokazuje na sledecu
		
		if(instr1==0x00 && instr2==0x00 && instr3==0x00 && instr4==0x00){
			break;
		}
		else if(instr1==0x10 && instr2==0x00 && instr3==0x00 && instr4==0x00){
			r[14]=r[14]-4; upisiPodatak32b(status,r[14]); // push status
			r[14]=r[14]-4; upisiPodatak32b(r[15],r[14]);  // push pc
			cause=4;
			status &= ~0x1;
			r[15]=handler;
		}
		else if(instr1==0x96){
			if ((static_cast<int64_t>(static_cast<uint64_t>(r[BBBB]))+static_cast<int64_t>(static_cast<uint64_t>(r[CCCC]))+static_cast<int64_t>(DDDD)) >= 0xFFFFFF00 ){
        //ILLEGAL_MACRO();
      } //ILI MANJE OD 0 onda ilegalno adresiranje
			unsigned int adPom = static_cast<unsigned int>(static_cast<int64_t>(static_cast<uint64_t>(r[BBBB]))+static_cast<int64_t>(static_cast<uint64_t>(r[CCCC]))+static_cast<int64_t>(DDDD)) ;
			
			switch(AAAA){
			case 0: status = citajPodatak32b(adPom);
					break;
			case 1: handler = citajPodatak32b(adPom);
					break;
			case 2: cause = citajPodatak32b(adPom);
					break;
			}
		}
		else if(instr1==0x21){
			if ((static_cast<int64_t>(static_cast<uint64_t>(r[AAAA]))+static_cast<int64_t>(static_cast<uint64_t>(r[BBBB]))+static_cast<int64_t>(DDDD)) >= 0xFFFFFF00 ){
       // ILLEGAL_MACRO();
      } //ILI MANJE OD 0 onda ilegalno adresiranje
			unsigned int adPom = static_cast<unsigned int>(static_cast<int64_t>(static_cast<uint64_t>(r[AAAA]))+static_cast<int64_t>(static_cast<uint64_t>(r[BBBB]))+static_cast<int64_t>(DDDD)) ;
			
			r[14]=r[14]-4; upisiPodatak32b(r[15],r[14]);  // push pc
			r[15]=citajPodatak32b(adPom);
		}
		else if(instr1==0x93){
			unsigned int adPom = static_cast<unsigned int>(static_cast<int64_t>(static_cast<uint64_t>(r[BBBB])) + static_cast<int64_t>(DDDD)) ;
			if(r[BBBB]>=0xFFFFFF00){
        //ILLEGAL_MACRO();
      }

			r[AAAA]=citajPodatak32b(r[BBBB]); 
      std::cerr<<"upis u"<<static_cast<unsigned int>(AAAA) <<std::endl;
			r[BBBB]= adPom;             
      std::cerr<<"upis u"<< static_cast<unsigned int>(BBBB)<<std::endl;
		}
		else if(instr1==0x38){
      if(static_cast<unsigned int>(static_cast<int64_t>(static_cast<uint64_t>(r[AAAA])) + static_cast<int64_t>(DDDD)) >= 0xFFFFFF00){
        //ILLEGAL_MACRO();
      }
			unsigned int adPom = static_cast<unsigned int>(static_cast<int64_t>(static_cast<uint64_t>(r[AAAA])) + static_cast<int64_t>(DDDD));
			
			r[15]=citajPodatak32b(adPom);
		}
		else if(instr1==0x39){
			if (r[BBBB]==r[CCCC]){
        if(static_cast<unsigned int>(static_cast<int64_t>(static_cast<uint64_t>(r[AAAA])) + static_cast<int64_t>(DDDD)) >= 0xFFFFFF00) {
          //ILLEGAL_MACRO();
          }
				unsigned int adPom = static_cast<unsigned int>(static_cast<int64_t>(static_cast<uint64_t>(r[AAAA])) + static_cast<int64_t>(DDDD));
				r[15]=citajPodatak32b(adPom);
			}
		}
		else if(instr1==0x3A){
			if (r[BBBB]!=r[CCCC]){
        if(static_cast<unsigned int>(static_cast<int64_t>(static_cast<uint64_t>(r[AAAA])) + static_cast<int64_t>(DDDD)) >= 0xFFFFFF00){
          //ILLEGAL_MACRO();
        }
				unsigned int adPom = static_cast<unsigned int>(static_cast<int64_t>(static_cast<uint64_t>(r[AAAA])) + static_cast<int64_t>(DDDD));
				r[15]=citajPodatak32b(adPom);
			}
		}
		else if(instr1==0x3B){
			if ( static_cast<signed int>(r[BBBB]) > static_cast<signed int>(r[CCCC]) ){
        if(static_cast<unsigned int>(static_cast<int64_t>(static_cast<uint64_t>(r[AAAA])) + static_cast<int64_t>(DDDD)) >= 0xFFFFFF00){
         // ILLEGAL_MACRO();
        }
				unsigned int adPom = static_cast<unsigned int>(static_cast<int64_t>(static_cast<uint64_t>(r[AAAA])) + static_cast<int64_t>(DDDD));
				r[15]=citajPodatak32b(adPom);
			}
		}
		else if(instr1==0x81){
			if ( static_cast<int64_t>(static_cast<uint64_t>(r[AAAA])) + static_cast<int64_t>(DDDD) >= 0xFFFFFFFF ); //ILI MANJE OD 0 onda ilegalno adresiranje
			unsigned int adPom = static_cast<unsigned int>(static_cast<int64_t>(static_cast<uint64_t>(r[AAAA])) + static_cast<int64_t>(DDDD)) ;
			
			r[AAAA]=adPom;  
      std::cerr<<"upis u"<<static_cast<unsigned int>(AAAA) <<std::endl;
			upisiPodatak32b(r[CCCC],r[AAAA]);
		}
		else if(instr1==0x93){
      if (r[BBBB]>=0xFFFFFF00){
        //ILLEGAL_MACRO();
        }
			r[AAAA]=citajPodatak32b(r[BBBB]);
      std::cerr<<"upis u"<<static_cast<unsigned int>(AAAA) <<std::endl;
			
			unsigned int adPom = static_cast<unsigned int>(static_cast<int64_t>(static_cast<uint64_t>(r[BBBB])) + static_cast<int64_t>(DDDD)) ;
			
			r[BBBB]=adPom;
		}
		else if(instr1==0x40){
			unsigned int temp;
			temp=r[BBBB];	r[BBBB]=r[CCCC]; r[CCCC]=temp;
      std::cerr<<"upis u"<<static_cast<unsigned int>(BBBB)<<"  "<<static_cast<unsigned int>(CCCC) <<std::endl;
		}
		else if(instr1==0x50){
			r[AAAA]=r[BBBB]+r[CCCC]; 
      std::cerr<<"upis u"<< static_cast<unsigned int>(AAAA)<<std::endl;
		}
		else if(instr1==0x51){
			r[AAAA]=r[BBBB]-r[CCCC];
      std::cerr<<"upis u"<<static_cast<unsigned int>(AAAA) <<std::endl;
		}
		else if(instr1==0x52){
			r[AAAA]=r[BBBB]*r[CCCC];
      std::cerr<<"upis u"<< static_cast<unsigned int>(AAAA)<<std::endl;
		}
		else if(instr1==0x53){
			r[AAAA]=r[BBBB]/r[CCCC];
      std::cerr<<"upis u"<<static_cast<unsigned int>(AAAA) <<std::endl;
		}
		else if(instr1==0x60){
			r[AAAA]=~r[BBBB]; 
      std::cerr<<"upis u"<< static_cast<unsigned int>(AAAA)<<std::endl;
		}
		else if(instr1==0x61){
			r[AAAA]=r[BBBB] & r[CCCC];
      std::cerr<<"upis u"<<static_cast<unsigned int>(AAAA) <<std::endl;
		}
		else if(instr1==0x62){
			r[AAAA]=r[BBBB] | r[CCCC];
      std::cerr<<"upis u"<< static_cast<unsigned int>(AAAA)<<std::endl;
		}
		else if(instr1==0x63){
			r[AAAA]=r[BBBB] ^ r[CCCC];
      std::cerr<<"upis u"<< static_cast<unsigned int>(AAAA)<<std::endl;
		}
		else if(instr1==0x70){
			r[AAAA]=r[BBBB] << r[CCCC];
      std::cerr<<"upis u"<< static_cast<unsigned int>(AAAA)<<std::endl;
		}
		else if(instr1==0x71){
			r[AAAA]=r[BBBB] >> r[CCCC];
      std::cerr<<"upis u"<<static_cast<unsigned int>(AAAA) <<std::endl;
		}
		else if(instr1==0x92){
			if ( static_cast<int64_t>(static_cast<uint64_t>(r[BBBB])) + static_cast<int64_t>(static_cast<uint64_t>(r[CCCC])) + static_cast<int64_t>(DDDD) >= 0xFFFFFF00 ){
       // ILLEGAL_MACRO();
      } //ILI MANJE OD 0 onda ilegalno adresiranje
			unsigned int adPom = static_cast<unsigned int>(static_cast<int64_t>(static_cast<uint64_t>(r[BBBB])) + static_cast<int64_t>(static_cast<uint64_t>(r[CCCC])) + static_cast<int64_t>(DDDD)) ;
			
			r[AAAA]=citajPodatak32b(adPom);
      std::cerr<<"upis u"<<static_cast<unsigned int>(AAAA) <<std::endl;
		}
		else if(instr1==0x91){
			r[AAAA]=static_cast<unsigned int>(static_cast<int64_t>(static_cast<uint64_t>(r[BBBB]))+static_cast<int64_t>(DDDD));
      std::cerr<<"upis u"<<static_cast<unsigned int>(AAAA) <<std::endl;
		}
		else if(instr1==0x82){
			if ((static_cast<int64_t>(static_cast<uint64_t>(r[AAAA]))+static_cast<int64_t>(static_cast<uint64_t>(r[BBBB]))+static_cast<int64_t>(DDDD)) >= 0xFFFFFF00 ){
        //ILLEGAL_MACRO();
      } //ILI MANJE OD 0 onda ilegalno adresiranje
			unsigned int adPom = (static_cast<int64_t>(static_cast<uint64_t>(r[AAAA]))+static_cast<int64_t>(static_cast<uint64_t>(r[BBBB]))+static_cast<int64_t>(DDDD)) ;
			// mem[mem[adPom]]<=gprC
			unsigned int temp = citajPodatak32b(adPom);
			// mem[temp]<=gprC
			
			upisiPodatak32b(r[CCCC],temp);
		}
		else if(instr1==0x80){
			if ((static_cast<int64_t>(static_cast<uint64_t>(r[AAAA]))+static_cast<int64_t>(static_cast<uint64_t>(r[BBBB]))+static_cast<int64_t>(DDDD)) >= 0xFFFFFF00 ); //ILI MANJE OD 0 onda ilegalno adresiranje
			unsigned int adPom = (static_cast<int64_t>(static_cast<uint64_t>(r[AAAA]))+static_cast<int64_t>(static_cast<uint64_t>(r[BBBB]))+static_cast<int64_t>(DDDD)) ;
			
			upisiPodatak32b(r[CCCC],adPom);
		}
		else if(instr1==0x90){
      std::cerr<<"upis u"<<static_cast<unsigned int>(AAAA) <<std::endl;
			switch(BBBB){
			case 0: r[AAAA]= status;
					break;
			case 1: r[AAAA]= handler;
					break;
			case 2: r[AAAA]= cause;
					break;
			}
		}
		else if(instr1==0x94){
			switch(AAAA){
			case 0: status = r[BBBB];
					break;
			case 1: handler = r[BBBB];
					break;
			case 2: cause = r[BBBB];
					break;
			}
		}
		
		else {
			// nelegalna instrukcija
			// vrb treba push status; push pc; cause<=1; status<=status&(0xF); pc<=handle;
      r[14]=r[14]-4; upisiPodatak32b(status,r[14]); // push status
      r[14]=r[14]-4; upisiPodatak32b(r[15],r[14]);  // push pc
      cause=1;
      status &= 0x7;
      r[15]=handler;

			std::cerr << "Greška u citanju inst" << std::endl;
			return 1; // Vraćamo kod greške
		}
		
		
	}
	
	std::cout<< std::endl;
	std::cout <<"---------------------------------------------------------------------"<< std::endl;
	std::cout <<"Emulated processor executed halt instruction"<< std::endl;
  std::cout <<"Emulated processor state:"<< std::endl;
	
	// ovde ispis registara 
	for (int i=0;i<16;i++){
    if(i%4==0 && i!=0) std::cout<<std::endl;
	  std::cout <<"r"<<std::dec<< i<<"=0x"<<std::setw(8) << std::setfill('0') <<std::hex <<r[i] <<"      ";

}
std::cout<<std::endl;
	

	
	for(EmulMem* mem : MEMORIJA){
		delete mem;
	}
	
	return 0;
}



 // FUNKCIJE 
unsigned char dohvatiInstr(unsigned int adresa){
	//bool postoji=false;
	for (EmulMem* em : MEMORIJA){
		if (em->adr==adresa){
			//postoji=true;
			return em->sadrzaj;
		}
	}
	//if (postoji==false){
		EmulMem* noviBajt = new EmulMem();
		noviBajt->adr=adresa;
		noviBajt->sadrzaj=0x00;
			
		MEMORIJA.push_back(noviBajt);
		return noviBajt->sadrzaj;
	//}
}

void upisiPodatak32b(unsigned int podatak,unsigned int adresa){
	unsigned char pod1,pod2,pod3,pod4;
	bool p1p=false,p2p=false,p3p=false,p4p=false;
	
	pod1=static_cast<unsigned char>(podatak&0xFF);
	pod2=static_cast<unsigned char>((podatak>>8)&0xFF);
	pod3=static_cast<unsigned char>((podatak>>16)&0xFF);
	pod4=static_cast<unsigned char>((podatak>>24)&0xFF);
	
	for (EmulMem* em : MEMORIJA){		
		if (em->adr==adresa){
			em->sadrzaj=pod1;	p1p=true;	
		}
		if (em->adr==adresa+1){
			em->sadrzaj=pod2;	p2p=true;
		}
		if (em->adr==adresa+2){
			em->sadrzaj=pod3;	p3p=true;
		}
		if (em->adr==adresa+3){
			em->sadrzaj=pod4;	p4p=true;
		}
		
	}	
		if(p1p==false){
			EmulMem* noviBajt = new EmulMem();
			noviBajt->adr=adresa;
			noviBajt->sadrzaj=pod1;
			
			MEMORIJA.push_back(noviBajt);
		}
		if(p2p==false){
			EmulMem* noviBajt = new EmulMem();
			noviBajt->adr=adresa+1;
			noviBajt->sadrzaj=pod2;
			
			MEMORIJA.push_back(noviBajt);
		}
		if(p3p==false){
			EmulMem* noviBajt = new EmulMem();
			noviBajt->adr=adresa+2;
			noviBajt->sadrzaj=pod3;
			
			MEMORIJA.push_back(noviBajt);
		}
		if(p4p==false){
			EmulMem* noviBajt = new EmulMem();
			noviBajt->adr=adresa+3;
			noviBajt->sadrzaj=pod4;
			
			MEMORIJA.push_back(noviBajt);
		}	
	
}

unsigned int citajPodatak32b(unsigned int adresa){
	unsigned char pod1,pod2,pod3,pod4;
	unsigned int rezultat;
	bool p1p=false,p2p=false,p3p=false,p4p=false;
	
	for (EmulMem* em : MEMORIJA){		
		if (em->adr==adresa){
			pod1=em->sadrzaj;	p1p=true;	
		}
		if (em->adr==adresa+1){
			pod2=em->sadrzaj;	p2p=true;
		}
		if (em->adr==adresa+2){
			pod3=em->sadrzaj;	p3p=true;
		}
		if (em->adr==adresa+3){
			pod4=em->sadrzaj;	p4p=true;
		}
	}
	
		if(p1p==false){
			EmulMem* noviBajt = new EmulMem();
			noviBajt->adr=adresa;
			noviBajt->sadrzaj=0;
			pod1=0;
			
			MEMORIJA.push_back(noviBajt);
		}
		if(p2p==false){
			EmulMem* noviBajt = new EmulMem();
			noviBajt->adr=adresa+1;
			noviBajt->sadrzaj=0;
			pod2=0;
			
			MEMORIJA.push_back(noviBajt);
		}
		if(p3p==false){
			EmulMem* noviBajt = new EmulMem();
			noviBajt->adr=adresa+2;
			noviBajt->sadrzaj=0;
			pod3=0;
			
			MEMORIJA.push_back(noviBajt);
		}
		if(p4p==false){
			EmulMem* noviBajt = new EmulMem();
			noviBajt->adr=adresa+3;
			noviBajt->sadrzaj=0;
			pod4=0;
			
			MEMORIJA.push_back(noviBajt);
		}
		
	
	rezultat = (static_cast<unsigned int>(pod4) << 24) |
			   (static_cast<unsigned int>(pod3) << 16) |
			   (static_cast<unsigned int>(pod2) << 8)  |
			   static_cast<unsigned int>(pod1);
	return rezultat;
	
	
}