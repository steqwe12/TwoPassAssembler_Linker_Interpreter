#include <iostream>
#include <fstream>
#include <list>
#include <cstdint>
#include <string>
#include <cstring>
#include <vector>
#include <iomanip>
#include <stdexcept>

class TabSimb{
	public:
	int num; // simboli pocinju od 1
	unsigned int value;
	int type; // 1-NOTYP 2-SCTN
	int bind; // 1-LOC 2-GLOB
	int ndx; // 0-UND, ostalo normalno 1,2,3,..
	char* name;

	int pripFajlu;

  ~TabSimb(){delete [] name;}
};

class RelZap{
	public:
	char* sekcija; // .rela.text : ali radim samo .text a ispisujem .rela.text
	
	unsigned int offset;		// kasnije se pretvara u pravu adresu(tako sam izabrao jer mi se cini da je lakse odratiti relokacione zapise)
	int type; // msm da mi treba samo R_32_32=1
	char* simbol; // vrednost name iz TabSimb
	int addend;

	int pripFajlu;

  ~RelZap(){delete []sekcija;delete []simbol;}
};

class SadrSekc{
	public:
	char* sekcija; // .text
	
	unsigned int adr;
	unsigned char sadrzaj;

	int pripFajlu;

  ~SadrSekc(){delete []sekcija;} 
};

class SectionDefinition{
	public:
	char *name;
	unsigned int velicinaSekcije;
	unsigned int brojRelokacZapisa;

	int pripFajlu;
	
	unsigned int smestenaAdresa;
	bool smestena;

  ~SectionDefinition(){delete []name;}
};

class ZadatPlace{
  public:
  char *sekcija;
  unsigned int adresa;
  
  unsigned int velicUBajtovima;

  ~ZadatPlace(){delete []sekcija;}		// kada se citaju na ulazu placovi mora da se alocira naziv odnosno char *sekcija;
};




int main(int argc, char* argv[]) {   
  std::list<TabSimb*> TABELA_SIMBOLA=std::list<TabSimb*>();
  std::list<RelZap*> RELOKACIONI_ZAPISI=std::list<RelZap*>();
  std::list<SadrSekc*> SADRZAJ_SEKCIJA=std::list<SadrSekc*>();
  std::list<SectionDefinition*> SEKCIJE=std::list<SectionDefinition*>();
  //std::list<char*> POMOCNI_NIZOVI_CHAROVA=std::list<char*>();   ne treba mi
  std::list<ZadatPlace*> PLACEOVI=std::list<ZadatPlace*>();
  
  unsigned int prvaSlobodnaZaSmestanje=0;
	
	
	int numOfHex=0;
	int numOfOutFiles=0;
	int brojFajlova=0; // izbrojim po ulaznim argumentima 
	std::vector<std::pair<std::string, int>> ulazniFajlovi; // ne treba dealokacija
	
	std::string izlazLinkera;
	
	for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);

        // Obrada opcije -o
        if (arg == "-o") {
			
			if(brojFajlova>0) {
                std::cerr << "Greška: naveden ulazni pa argument" << std::endl;
                return 1; // Vraćamo kod greške
            }
			
            if (i + 1 < argc) {
				numOfOutFiles++;
                std::string izlaznaDatoteka(argv[i + 1]);
                //std::cout << "Izlazna datoteka: " << izlaznaDatoteka << std::endl;
				izlazLinkera=izlaznaDatoteka;
                i++; // Preskačemo sledeći argument jer smo ga već obradili
            } else {
                std::cerr << "Greška: Nedostaje argument za opciju -o." << std::endl;
                return 1; // Vraćamo kod greške
            }
        }
        // Obrada opcije --place=<ime_sekcije>@<adresa>
        else if (arg.find("-place=") == 0) {
			if(brojFajlova>0) {
                std::cerr << "Greška: naveden ulazni pa argument" << std::endl;
                return 1; // Vraćamo kod greške
            }
			
			std::string argument = arg.substr(7);
            int found = argument.find('@');
            if (found != std::string::npos) {
                std::string imeSekcije = argument.substr(0, found);
                std::string adresa = argument.substr(found + 1);
				
        size_t idx;
				uint64_t adr = std::stoull(adresa, &idx, 0);

        if (idx < adresa.size()) {
            std::cerr << "Nije moguće konvertovati ceo string u broj" << std::endl;
                return 1; // Vraćamo kod greške
        }

				if (adr>0xFFFFFFFF){
					std::cerr << "Greška: place nevalidna adresa" << std::endl;
					return 1; // Vraćamo kod greške
				}
				
				// ovde treba da se prave PLACEOVI
				// kad se prave ZadatPlace-ovi proveri se da li vec postoji za tu sekciju, ako postoji GRESKA								
				for (ZadatPlace* zp : PLACEOVI){
					if (strcmp(zp->sekcija,imeSekcije.c_str())==0){
						std::cerr << "Greška: place zadat vise puta" << std::endl;
						return 1; // Vraćamo kod greške
					}
				}
							
				ZadatPlace* zadPl = new ZadatPlace();
				zadPl->adresa=static_cast<unsigned int>(adr);
				zadPl->sekcija=new char[imeSekcije.length() + 1];
				strcpy(zadPl->sekcija, imeSekcije.c_str());

				
				PLACEOVI.push_back(zadPl);
                
            } else {
                std::cerr << "Greška: Neispravna opcija --place." << std::endl;
                return 1; // Vraćamo kod greške
            }
        }
        // Obrada opcije -hex
        else if (arg == "-hex") {
			if(brojFajlova>0) {
                std::cerr << "Greška: naveden ulazni pa argument" << std::endl;
                return 1; // Vraćamo kod greške
            }
			
            numOfHex++;
        }
        else{
			brojFajlova++; ulazniFajlovi.push_back(std::make_pair(arg, brojFajlova));
		}

    }

std::cout<<"TESTESTTESTTTTTTTTTTT"<<std::endl;
   for(ZadatPlace* zp  : PLACEOVI){
    std::cout<<zp->sekcija <<"   ";
    std::cout<<zp->adresa <<"    ";
    std::cout<<zp->velicUBajtovima ;

    std::cout<<std::endl;
   }
   std::cout<<std::endl;
std::cout<<"TESTESTTESTTTTTTTTTTT"<<std::endl;
	
	if(numOfHex!=1) return 0;
	if(numOfOutFiles!=1) {std::cerr<<"sme samo jedna -o naziv_datoteke opcija "<< std::endl ;return 1;}
	
	for (const auto& par : ulazniFajlovi) {									// PETLJA ZA CITANJE SVIH ULAZNIH FAJLOVA
       // par.first ;par.second ;       ovde da procitam sve fajlove i strukturama dodeljujem pripFajlu=par.second
	   std::ifstream binarniFajl(par.first, std::ios::binary);
	   
	   if (binarniFajl.is_open()) {
        //char* ocekivanaSekvenca = new char[5];ocekivanaSekvenca[0]='E';ocekivanaSekvenca[1]='L';ocekivanaSekvenca[2]='F';ocekivanaSekvenca[3]='2';
        //ocekivanaSekvenca[4]='\0';
        //"ELF2";
    //char* procitanaSekvenca;

    //binarniFajl.read(procitanaSekvenca, sizeof(ocekivanaSekvenca));

    // Provera da li je pročitana sekvence jednaka očekivanoj
    //if (strcmp(procitanaSekvenca, ocekivanaSekvenca) != 0) {
     //   std::cerr << ocekivanaSekvenca<<"   "<<procitanaSekvenca<< "Fajl nije u očekivanom formatu." << std::endl;
     //   return 1;
	//}


    unsigned int brojElemenata;
    binarniFajl.read(reinterpret_cast<char*>(&brojElemenata), sizeof(brojElemenata));

    for (int i=0;i<brojElemenata;i++){
      TabSimb* simbol = new TabSimb();

      // Čitanje podataka za svaki simbol iz binarnog fajla
    binarniFajl.read(reinterpret_cast<char*>(&simbol->num), sizeof(simbol->num));
    binarniFajl.read(reinterpret_cast<char*>(&simbol->value), sizeof(simbol->value));
    binarniFajl.read(reinterpret_cast<char*>(&simbol->type), sizeof(simbol->type));
    binarniFajl.read(reinterpret_cast<char*>(&simbol->bind), sizeof(simbol->bind));
    binarniFajl.read(reinterpret_cast<char*>(&simbol->ndx), sizeof(simbol->ndx));

    // Čitanje veličine niza karaktera 'name'
    unsigned int velicinaImena;
    binarniFajl.read(reinterpret_cast<char*>(&velicinaImena), sizeof(velicinaImena));

    // Čitanje samog niza karaktera 'name'
    simbol->name = new char[velicinaImena + 1];
    binarniFajl.read(simbol->name, velicinaImena);
    simbol->name[velicinaImena] = '\0';  // Postavljanje null terminatora

	simbol->pripFajlu=par.second;

    TABELA_SIMBOLA.push_back(simbol);

    }

    unsigned int brojSekcija;
    binarniFajl.read(reinterpret_cast<char*>(&brojSekcija), sizeof(brojSekcija));


for (unsigned int i = 0; i < brojSekcija; ++i) {
    SectionDefinition* sekcija = new SectionDefinition();  // Pretpostavljamo da imate odgovarajuću klasu za sekciju

    // Čitanje podataka za svaku sekciju iz binarnog fajla
    binarniFajl.read(reinterpret_cast<char*>(&sekcija->velicinaSekcije), sizeof(sekcija->velicinaSekcije));
    binarniFajl.read(reinterpret_cast<char*>(&sekcija->brojRelokacZapisa), sizeof(sekcija->brojRelokacZapisa));

    // Čitanje veličine niza karaktera 'name' sekcije
    unsigned int velicinaImena;
    binarniFajl.read(reinterpret_cast<char*>(&velicinaImena), sizeof(velicinaImena));

    // Čitanje samog niza karaktera 'name' sekcije
    sekcija->name = new char[velicinaImena + 1];
    binarniFajl.read(sekcija->name, velicinaImena);
    sekcija->name[velicinaImena] = '\0';  // Postavljanje null terminatora

	sekcija->pripFajlu=par.second;

    SEKCIJE.push_back(sekcija);

    unsigned int brojacAdr=0;
    for (unsigned int j = 0; j < sekcija->velicinaSekcije; ++j) {
        unsigned char podatak;
        binarniFajl.read(reinterpret_cast<char*>(&podatak), sizeof(podatak));

        SadrSekc *sadrSekc = new SadrSekc();

        size_t length = std::strlen(sekcija->name);
        sadrSekc->sekcija = new char[length + 1];
        std::strcpy(sadrSekc->sekcija, sekcija->name);

        sadrSekc->adr=brojacAdr++;
        sadrSekc->sadrzaj=podatak;

		sadrSekc->pripFajlu=par.second;

        SADRZAJ_SEKCIJA.push_back(sadrSekc);
    }

    for (unsigned int k = 0; k < sekcija->brojRelokacZapisa; ++k) {
        RelZap* relokZapis = new RelZap();  // Pretpostavljamo da imate odgovarajuću klasu za relokacioni zapis

        // Čitanje podataka za svaki relokacioni zapis iz binarnog fajla
        binarniFajl.read(reinterpret_cast<char*>(&relokZapis->offset), sizeof(relokZapis->offset));
        binarniFajl.read(reinterpret_cast<char*>(&relokZapis->type), sizeof(relokZapis->type));

        // Čitanje veličine niza karaktera 'simbol' relokacionog zapisa
        unsigned int velicinaImenaRelZap;
        binarniFajl.read(reinterpret_cast<char*>(&velicinaImenaRelZap), sizeof(velicinaImenaRelZap));

        // Čitanje samog niza karaktera 'simbol' relokacionog zapisa
        relokZapis->simbol = new char[velicinaImenaRelZap + 1];
        binarniFajl.read(relokZapis->simbol, velicinaImenaRelZap);
        relokZapis->simbol[velicinaImenaRelZap] = '\0';  // Postavljanje null terminatora

        binarniFajl.read(reinterpret_cast<char*>(&relokZapis->addend), sizeof(relokZapis->addend));

        size_t length = std::strlen(sekcija->name);
        relokZapis->sekcija = new char[length + 1];
        std::strcpy(relokZapis->sekcija, sekcija->name);
		
		    relokZapis->pripFajlu=par.second;
        
        RELOKACIONI_ZAPISI.push_back(relokZapis);
    }
}
        binarniFajl.close();
    } else {
        std::cerr << "Nije moguće otvoriti fajl." << std::endl;
    }	   
    }																// KRAJ PETLJE ZA CITANJE SVIH ULAZNIH FAJLOVA
	
	
	  
  
  // kad se prave ZadatPlace-ovi proveri se da li vec postoji za tu sekciju, ako postoji GRESKA
  // pretpostavka odvde je sve procitano iz svih fajlova i popunjene su FULL ove liste
  
  for(ZadatPlace* zadatPlace : PLACEOVI){
	  for (SectionDefinition* secDef : SEKCIJE){
		  if (strcmp(zadatPlace->sekcija,secDef->name)==0){
			  if (zadatPlace->adresa > zadatPlace->adresa+zadatPlace->velicUBajtovima){
				  std::cerr << "sekcije prelazi granicu:"<<zadatPlace->sekcija << std::endl;
				  std::exit(EXIT_FAILURE);
			  }
			  zadatPlace->velicUBajtovima = zadatPlace->velicUBajtovima + secDef->velicinaSekcije;			  
		  }
	  }
  }
  
  for(ZadatPlace* zadatPlace : PLACEOVI){
	  bool imePlacePostoji=false;
	  for (SectionDefinition* secDef : SEKCIJE){
		  	if (strcmp(zadatPlace->sekcija,secDef->name)==0)   imePlacePostoji=true;
	  }
	  if (imePlacePostoji==false){
		   std::cerr << "zadat place za nepostojacu sekciju"<<zadatPlace->sekcija << std::endl;
		   std::exit(EXIT_FAILURE);
	  }
  }
  
  
  // da li se sekcija preko -place=.. preklapaju
  for(ZadatPlace* zadatPlace1 : PLACEOVI){
	  for(ZadatPlace* zadatPlace2 : PLACEOVI){	  
		  if(strcmp(zadatPlace1->sekcija,zadatPlace2->sekcija)!=0){
			  
			if(zadatPlace2->adresa>=zadatPlace1->adresa && (zadatPlace2->adresa+zadatPlace2->velicUBajtovima)<=(zadatPlace1->adresa+zadatPlace1->velicUBajtovima)){
				std::cerr << "sekcije se preklapaju1:"<<zadatPlace1->sekcija<<", "<<zadatPlace2->sekcija << std::endl;
				std::exit(EXIT_FAILURE);
			}
			else if(zadatPlace2->adresa<=zadatPlace1->adresa && (zadatPlace2->adresa+zadatPlace2->velicUBajtovima)>=(zadatPlace1->adresa+zadatPlace1->velicUBajtovima)){
				std::cerr << "sekcije se preklapaju2:"<<zadatPlace1->sekcija<<", "<<zadatPlace2->sekcija << std::endl;
				std::exit(EXIT_FAILURE);
			}
			else if(zadatPlace2->adresa>=zadatPlace1->adresa && (zadatPlace2->adresa)<(zadatPlace1->adresa+zadatPlace1->velicUBajtovima)){  // mozda <=
				std::cerr << "sekcije se preklapaju3:"<<zadatPlace1->sekcija<<", "<<zadatPlace2->sekcija << std::endl;
				std::exit(EXIT_FAILURE);
			}
			else if((zadatPlace2->adresa+zadatPlace2->velicUBajtovima)>zadatPlace1->adresa && (zadatPlace2->adresa+zadatPlace2->velicUBajtovima)<=(zadatPlace1->adresa+zadatPlace1->velicUBajtovima)){
				std::cerr << "sekcije se preklapaju4:"<<zadatPlace1->sekcija<<", "<<zadatPlace2->sekcija << std::endl;
				std::exit(EXIT_FAILURE);
			}
    }
		  
	  }
  }
  
  // treba da se proveri da li sekcije mogu da stanu GORE, odnosno kad se sve stave da ne budu vece od 0xFFFFFFFF; memorijski mapirani reg se proveravaju u emulatoru
  
  // ovde za visestruke def simbola & nerazresene simb
  for (TabSimb* simbol1 : TABELA_SIMBOLA){
	  for (TabSimb* simbol2 : TABELA_SIMBOLA){
		if(simbol1->pripFajlu!=simbol2->pripFajlu && simbol1->type!=2 && simbol2->type!=2 && strcmp(simbol1->name,simbol2->name)==0){  	// type=2 je za sekcije
			if(simbol1->bind==2 && simbol2->bind==2 && simbol1->ndx>0 && simbol2->ndx>0){
				std::cerr << "visestruka def simbola:"<<simbol1->name<<", "<<simbol2->name << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
	  }
  }
  
  for (TabSimb* simbol1 : TABELA_SIMBOLA){
	  if(simbol1->type!=2 && simbol1->bind==2 && simbol1->ndx==0){
		  bool defNegde=false;
		  for (TabSimb* simbol2 : TABELA_SIMBOLA){
			if(simbol1->pripFajlu!=simbol2->pripFajlu && simbol2->type!=2 && simbol2->bind==2 && simbol2->ndx>0 && strcmp(simbol1->name,simbol2->name)==0) defNegde=true;
		  }
		  if (defNegde==false){
			  std::cerr << "nerazresen simbol:"<<simbol1->name<< std::endl;
			  std::exit(EXIT_FAILURE);
		  }		  
	  }
  }
  
  
  for (ZadatPlace* zadPlace : PLACEOVI){
	  if ((zadPlace->adresa+zadPlace->velicUBajtovima)>prvaSlobodnaZaSmestanje) prvaSlobodnaZaSmestanje=zadPlace->adresa+zadPlace->velicUBajtovima;    // nadjemo adresa od koje se smestaju sekcije bez place-a
  }
  
  
  
  for (SectionDefinition* secDef : SEKCIJE){
	  bool postoji=false;
	  for(ZadatPlace* zadPlace : PLACEOVI){
		  if (strcmp(secDef->name,zadPlace->sekcija)==0) postoji=true;
	  }
	  
	  if (postoji==false){	// pravimo mapiranje sekcije
		  ZadatPlace* zadPlace = new ZadatPlace();
		  unsigned int velic=0;
		  
		  for(SectionDefinition* sD : SEKCIJE){
			  if (strcmp(secDef->name,sD->name)==0) {
				  velic=velic+sD->velicinaSekcije;
			  }
		  }
		  
		  zadPlace->velicUBajtovima=velic;
		  zadPlace->adresa=prvaSlobodnaZaSmestanje;
		  prvaSlobodnaZaSmestanje=prvaSlobodnaZaSmestanje+velic;
		  
		  size_t length = std::strlen(secDef->name);
		  zadPlace->sekcija=new char[length+1];
		  std::strcpy(zadPlace->sekcija, secDef->name);		  									
		  
		  PLACEOVI.push_back(zadPlace);
	  }
  }
  
 for(ZadatPlace* zadatPlace : PLACEOVI){
	  for (SectionDefinition* secDef : SEKCIJE){
		  if (strcmp(zadatPlace->sekcija,secDef->name)==0){
			  if (zadatPlace->adresa > zadatPlace->adresa+zadatPlace->velicUBajtovima){
				  std::cerr << "sekcije prelazi granicu:"<<zadatPlace->sekcija << std::endl;
				  std::exit(EXIT_FAILURE);
			  }
			  zadatPlace->velicUBajtovima = zadatPlace->velicUBajtovima + secDef->velicinaSekcije;			  
		  }
	  }
  }

  // da li se sekcija preklapaju sve iz ZadatPlace koje sam mapirao,a sve sam ih mapirao
  for(ZadatPlace* zadatPlace1 : PLACEOVI){
	  for(ZadatPlace* zadatPlace2 : PLACEOVI){	  
		  if(strcmp(zadatPlace1->sekcija,zadatPlace2->sekcija)!=0){
			  
			if(zadatPlace2->adresa>=zadatPlace1->adresa && (zadatPlace2->adresa+zadatPlace2->velicUBajtovima)<=(zadatPlace1->adresa+zadatPlace1->velicUBajtovima)){
				std::cerr << "sekcije se preklapaju1:"<<zadatPlace1->sekcija<<", "<<zadatPlace2->sekcija << std::endl;
				std::exit(EXIT_FAILURE);
			}
			else if(zadatPlace2->adresa<=zadatPlace1->adresa && (zadatPlace2->adresa+zadatPlace2->velicUBajtovima)>=(zadatPlace1->adresa+zadatPlace1->velicUBajtovima)){
				std::cerr << "sekcije se preklapaju2:"<<zadatPlace1->sekcija<<", "<<zadatPlace2->sekcija << std::endl;
				std::exit(EXIT_FAILURE);
			}
			else if(zadatPlace2->adresa>=zadatPlace1->adresa && (zadatPlace2->adresa)<(zadatPlace1->adresa+zadatPlace1->velicUBajtovima)){  // mozda <=
				std::cerr << "sekcije se preklapaju3:"<<zadatPlace1->sekcija<<", "<<zadatPlace2->sekcija << std::endl;
				std::exit(EXIT_FAILURE);
			}
			else if((zadatPlace2->adresa+zadatPlace2->velicUBajtovima)>zadatPlace1->adresa && (zadatPlace2->adresa+zadatPlace2->velicUBajtovima)<=(zadatPlace1->adresa+zadatPlace1->velicUBajtovima)){
				std::cerr << "sekcije se preklapaju4:"<<zadatPlace1->sekcija<<", "<<zadatPlace2->sekcija << std::endl;
				std::exit(EXIT_FAILURE);
			}
    }
		  
	  }
  }










  
  // za sada sam mapirao sve sekcije ali spojene( ZadatPlace ), tek treba npr .text(1) .text(2) da mapiram tacno
  
  // ovde mapiram tacno sekcije
  for (ZadatPlace* zadatPlace : PLACEOVI) {
	  
	  unsigned int pocAd=zadatPlace->adresa;
	  
	  for (int i=1;i<brojFajlova+1;i++){
		  
		  for (SectionDefinition* sD : SEKCIJE){
			  if (sD->pripFajlu==i && strcmp(zadatPlace->sekcija,sD->name)==0){
				  sD->smestenaAdresa=pocAd;
				  sD->smestena=true;
				  pocAd=pocAd+sD->velicinaSekcije;
			  }
		  }
		  
	  }
  }
  
  
  // za svaki SectionDefinition smestenaAdresa-e su odredjenje, odnosno i .text(1) .text(2) itd itd su mapirani svi i zna se iz kojih su fajlova
  
  
  
  for (SectionDefinition* secDef : SEKCIJE){
	  for (SadrSekc* sadrSekc : SADRZAJ_SEKCIJA){
		  if (secDef->pripFajlu==sadrSekc->pripFajlu && strcmp(secDef->name,sadrSekc->sekcija)==0){
			  sadrSekc->adr=sadrSekc->adr+secDef->smestenaAdresa;
		  }
	  }
  }
  
  // svi sadrzaji svih sekcija svih fajlova mapirani pravilno potpuno
  
  
  

  // ovde treba da odradim prave adrese u relokacionim zapisima
  for (RelZap* relZap : RELOKACIONI_ZAPISI){
	for (SectionDefinition* secDef : SEKCIJE){
		if (strcmp(relZap->sekcija,secDef->name)==0  && relZap->pripFajlu==secDef->pripFajlu){
			relZap->offset=relZap->offset+secDef->smestenaAdresa;
		}
	}
  }
  
  
  
  
  
  // odredjujem prave vrednosti simbola:
  for (TabSimb* tabSimb : TABELA_SIMBOLA){
	  if (tabSimb->bind==2 && tabSimb->ndx>0){
		  for (TabSimb* tabSimb2 : TABELA_SIMBOLA){
			  if (tabSimb2->type==2 && tabSimb->pripFajlu==tabSimb2->pripFajlu && tabSimb->ndx==tabSimb2->ndx){
				  //tabSimb2->name je ono sto zelimo- ime sekcije kojoj pripada simbol
				  for (SectionDefinition* secDef : SEKCIJE){
					  if (strcmp(secDef->name,tabSimb2->name)==0 && secDef->pripFajlu==tabSimb2->pripFajlu) {
						tabSimb->value=tabSimb->value+secDef->smestenaAdresa;
					  }
				  }
			  }
		  }
	  }
  }
  
  
  // obradim sada relokacione zapise sve, orderujem(ili pre) SadrSekc list po adr i napravim izlaz linkera
  for (RelZap* relZap : RELOKACIONI_ZAPISI){
	  //pretragra simbola
	  bool jesteObicanSimbol=false;
	  unsigned char prvi,drugi,treci,cetvrti;
	  for (TabSimb* tabSimb : TABELA_SIMBOLA){
		  if(strcmp(relZap->simbol,tabSimb->name)==0 && tabSimb->type==1  && tabSimb->bind==2  && tabSimb->ndx>0) {
			jesteObicanSimbol=true;
			int64_t zaUpisPom=(static_cast<int64_t>(static_cast<uint64_t>(tabSimb->value)))+static_cast<int64_t>(relZap->addend);							// ne treba provera jer vrednost za upis ne prelazi granicu vredn sekc+velic sekc
			unsigned int zaUpis=static_cast<unsigned int>(zaUpisPom);
			prvi=static_cast<unsigned char>(zaUpis&0xFF);
			drugi=static_cast<unsigned char>((zaUpis>>8)&0xFF);
			treci=static_cast<unsigned char>((zaUpis>>16)&0xFF);
			cetvrti=static_cast<unsigned char>((zaUpis>>24)&0xFF);
		  }
	  }
	  
	  if(jesteObicanSimbol==false){ // u pitanju je onda relokacija gde simbol ima vrednost sekcije npr .section .text
		  for(SectionDefinition* secDef : SEKCIJE){
			  if (strcmp(relZap->simbol,secDef->name)==0 && relZap->pripFajlu==secDef->pripFajlu){
				  int64_t zaUpisPom=(static_cast<int64_t>(static_cast<uint64_t>(secDef->smestenaAdresa)))+static_cast<int64_t>(relZap->addend);				// ne treba provera jer vrednost za upis ne prelazi granicu vredn sekc+velic sekc
				  unsigned int zaUpis=static_cast<unsigned int>(zaUpisPom);
				  prvi=static_cast<unsigned char>(zaUpis&0xFF);
				  drugi=static_cast<unsigned char>((zaUpis>>8)&0xFF);
				  treci=static_cast<unsigned char>((zaUpis>>16)&0xFF);
				  cetvrti=static_cast<unsigned char>((zaUpis>>24)&0xFF);
			  }
		  }
	  }
	  
	  
	  // ovde upisujem
	  for (SadrSekc* sadSekc : SADRZAJ_SEKCIJA){
		  if (sadSekc->adr==relZap->offset){
			  sadSekc->sadrzaj=prvi;
		  }
		  else if (sadSekc->adr==(relZap->offset+1)){
			  sadSekc->sadrzaj=drugi;
		  }
		  else if (sadSekc->adr==(relZap->offset+2)){
			  sadSekc->sadrzaj=treci;
		  }
		  else if (sadSekc->adr==(relZap->offset+3)){
			  sadSekc->sadrzaj=cetvrti;
		  }
		
	  }
  }
  
  // ovde da orderujem sve sadrzaje sekcija po adresi za svaki slucaj
  SADRZAJ_SEKCIJA.sort([](SadrSekc* a, SadrSekc* b) {
        return a->adr < b->adr;
    });
  
  
  
  // ovde upis u tekstualnu datoteku
  std::ofstream outFile("izlLink.hex");

    if (!outFile.is_open()) {
        std::cerr << "Unable to open file: izlLink.txt"<< std::endl;
        return 1;
    }

	int i=0;
	unsigned int pomAdr=0;
	unsigned int trenBaseAdr=0;
    // Iteracija kroz sve elemente u listi
    for (SadrSekc* a : SADRZAJ_SEKCIJA) {
		i++;
		if (i==1 && (a->adr % 8 != 0)){
			outFile << std::setw(4) << std::setfill('0') << std::hex << ((a->adr)-(a->adr % 8))<< ": ";
			for(unsigned int j=0;j<a->adr % 8;j++) outFile << "XX ";
			outFile << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned short>(a->sadrzaj) << ' ';
			trenBaseAdr=(a->adr)-(a->adr % 8);
		}else {
	
			// Ako smo stigli do kraja reda, dodajemo novi red
			if (a->adr % 8 == 0) {	// a->sadrzaj
				if (i!=1) {
					outFile << std::endl;
				}
				trenBaseAdr=a->adr;
				outFile << std::setw(4) << std::setfill('0') << std::hex << a->adr << ": ";
			}
			
			// Upisujemo sadržaj na datu adresu
			if(a->adr <= trenBaseAdr+7){
			outFile << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned short>(a->sadrzaj) << ' ';
			}		
			else{
				outFile << std::endl;
				trenBaseAdr=(a->adr)-(a->adr % 8);
				outFile << std::setw(4) << std::setfill('0') << std::hex << trenBaseAdr<< ": ";
				for(unsigned int j=0;j<a->adr % 8;j++) outFile << "XX ";
				outFile << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned short>(a->sadrzaj) << ' ';
			}
			
		}
    }

  outFile.close();					// kraj upisa tekstualne datoteke
  
  
  
  // ovde pravljenje binarne datoteke
  std::ofstream binarniFajl(izlazLinkera, std::ios::binary);
  if (!binarniFajl) {
        std::cerr << "Greška prilikom otvaranja" <<izlazLinkera<< "fajla!" << std::endl;
        return 1;
    }
  
  unsigned int brojSadrSekc = static_cast<unsigned int>(SADRZAJ_SEKCIJA.size()); 
  binarniFajl.write(reinterpret_cast<const char*>(&brojSadrSekc), sizeof(brojSadrSekc));
  
  for (SadrSekc* zapis : SADRZAJ_SEKCIJA){
	  binarniFajl.write(reinterpret_cast<const char*>(&zapis->adr), sizeof(zapis->adr));
      binarniFajl.write(reinterpret_cast<const char*>(&zapis->sadrzaj), sizeof(zapis->sadrzaj));
  }
  
  
  
  
  
  binarniFajl.close();		// zatvaranje binarne datoteke
  
  
  
  
  
  std::cout << std::dec << std::endl; // za svaki slucaj zbog std::hex
  
  std::cout << std::endl;std::cout << "ZadatPlace:";std::cout << std::endl;
  std::cout << "sekcija         adresa        velicUBajtovima  ";std::cout << std::endl;
  for(ZadatPlace* zp1 : PLACEOVI){
	  std::cout<<zp1->sekcija<<"         ";
    std::cout<< std::hex<<zp1->adresa<<"         ";
    std::cout<<std::dec <<  zp1->velicUBajtovima;
    std::cout << std::endl;
}



std::cout << std::endl;std::cout << "SectionDefinition:";std::cout << std::endl;
  std::cout << "name         velicinaSekcije        brojRelokacZapisa             pripFajlu          smestenaAdresa          smestena";std::cout << std::endl;
  for(SectionDefinition* sd1 : SEKCIJE){
	  std::cout<<sd1->name<<"           ";
    std::cout<< sd1->velicinaSekcije<<"                       ";
    std::cout<< sd1->brojRelokacZapisa<<"                         ";
	 std::cout<< sd1->pripFajlu<<"               ";
	  std::cout<< std::hex<<sd1->smestenaAdresa<<"                 ";
	   std::cout<< std::dec << sd1->smestena;
	
    std::cout << std::endl;
}


std::cout << std::endl;std::cout << "RelZap:";std::cout << std::endl;
  std::cout << "sekcija         offset        type             simbol          addend          pripFajlu";std::cout << std::endl;
  for(RelZap* rz1 : RELOKACIONI_ZAPISI){
	  std::cout<<rz1->sekcija<<"         ";
    std::cout<< std::hex<<rz1->offset<<"         ";
    std::cout<< std::dec << rz1->type<<"         ";
	 std::cout<< rz1->simbol<<"           ";
	  std::cout<< rz1->addend<<"               ";
	   std::cout<< rz1->pripFajlu;
	
    std::cout << std::endl;
}

std::cout << std::endl;std::cout << "TabSimb:";std::cout << std::endl;
  std::cout << "value         pripFajlu        name      ";std::cout << std::endl;
  for(TabSimb* rz1 : TABELA_SIMBOLA){
    if(rz1->ndx>0 && rz1->bind==2 && rz1->type==1){
      std::cout<<std::hex<<rz1->value<<"         ";
      std::cout<< std::dec <<rz1->pripFajlu<<"         ";
      std::cout<< rz1->name;
    
      std::cout << std::endl;
  }
}






  
  
  
 /* 																PRINT KAO IZLAZ ASEMBLERA
  
  // ovde print
// TABELA SIMBOLA
  std::cout << std::endl;std::cout << "TABELA SIMBOLA:";std::cout << std::endl;
  std::cout << "num   value     type   bind   ndx         name  ";std::cout << std::endl;
  for(TabSimb *simbol : TABELA_SIMBOLA){
	  std::cout<<simbol->num<<"      ";
    std::cout<< simbol->value<<"       ";
    std::cout<< simbol->type<<"       ";
    std::cout<< simbol->bind<<"       ";
    std::cout<< simbol->ndx<<"       ";
    std::cout<< simbol->name;
    std::cout << std::endl;
}
std::cout << std::endl;
// SVAKA SEKCIJA POJEDINACNO
for(SectionDefinition *sekcija : SEKCIJE){
  std::cout<<"******** "<< sekcija->name<<" ********";
  int i=0;
  for(SadrSekc *sadrzajSekc : SADRZAJ_SEKCIJA){
    if(strcmp(sekcija->name,sadrzajSekc->sekcija)==0){
      if(i%16==0) std::cout<<std::endl;
      else if(i%4==0) std::cout<<"       ";
      std::cout<< sadrzajSekc->adr;
      std::cout<< ": "<<std::hex << "0x" << static_cast<int>(sadrzajSekc->sadrzaj) << "  ";

      i++;
    }
  }
  std::cout << std::endl;std::cout << std::endl;std::cout<<"******* .rela " <<sekcija->name<<" ********";std::cout << std::endl;
  std::cout << "offset  type   symbol          addend"; std::cout << std::endl;
  for(RelZap *relZapis : RELOKACIONI_ZAPISI){
    if(strcmp(sekcija->name,relZapis->sekcija)==0){
      std::cout<<std::hex << "0x"<<relZapis->offset<<"       ";
      std::cout<< std::dec<< relZapis->type<<"       ";
      std::cout<< relZapis->simbol<<"       ";
      std::cout<< relZapis->addend;
      std::cout << std::endl;
    }
  }
  std::cout << std::endl;
}
  
  */
  
  
  
  
  // treba da dealociram i da pogledam destruktore da li se sve pravilno dealocira
  for(TabSimb* simbol : TABELA_SIMBOLA){
    delete simbol;
  }
   for(RelZap* relz : RELOKACIONI_ZAPISI){
    delete relz;
  }
   for(SadrSekc* simbol : SADRZAJ_SEKCIJA){
    delete simbol;
  }
   for(SectionDefinition* simbol : SEKCIJE){
    delete simbol;
  }
  for(ZadatPlace* simbol : PLACEOVI){
    delete simbol;
  }
  
  
  return 0; // zbog int main(..){..}
}