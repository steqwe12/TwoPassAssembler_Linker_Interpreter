#include "../inc/helpers.hpp"
//#include "lekser.hpp"
#include "../inc/parser.hpp"
#include "../inc/bitno.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

#include <list>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <iomanip>

extern FILE* yyin;

 //********************************************************************************************************************************

int main(int argc, char* argv[]){	

  std::streambuf* cerr_buffer = std::cerr.rdbuf();
  std::ofstream cerr_file("error_log.txt");
  std::cerr.rdbuf(cerr_file.rdbuf());

  if (argc != 4) {
        std::cerr << "Upotreba: " << argv[0] << " -o <naziv_izlazne_datoteke> <naziv_ulazne_datoteke>" << std::endl;
        return 1;
    }

  std::string opcija = argv[1];
  std::string nazivIzlazne = argv[2];
  std::string nazivUlazne = argv[3];

  if (opcija != "-o") {
        std::cerr << "Greška: Očekivana opcija -o." << std::endl;
        return 1;
    }

   FILE *myfile = fopen(argv[3], "r");
  // Make sure it is valid:
  if (!myfile) {
    printf("cant opennnnn ");
    return -1;
  }
  // Set Flex to read from it instead of defaulting to STDIN:
  yyin = myfile;

  yyparse();

  // RAD
  //ako sve radi, na kraju da raspodelim u foldere- inc,src,misc,makefile i izmenim sve tako da radi

  //*********************************************************************************************************************************

instr *obr_inst = global_instr;


	// DVA PROLAZA ASEMBLERA
for(int i=1;i<3;i++){
	obr_inst = global_instr;	// prvi i drugi prolaz
	locationCounter=0;
	poolCounter=0;
	curSect=nullptr;
	
	while(obr_inst!=nullptr){	// prolaz kroz direktive i instrukcije
	  const char* obrInstName = static_cast<const char*>(obr_inst->name);
		
    arg* argument=nullptr;

		if(strcmp(obrInstName,".word")==0){			
			switch(i){
				case 1:
					if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter); // samo za prvi .word arg,... se dodaje simbol
					
					//foreach args locationCounter=locationCounter+4;
					argument=obr_inst->args;
					while(argument!=nullptr){
						locationCounter=locationCounter+4;
						argument=argument->next;
					}
					break;
				case 2:
					//numerisiIndxSIMBOL(obr_inst->label); mozda ipak na kraju 2. prolaza sve simbole u tabsimb osim sekcija
					
					argument=obr_inst->args;
					while(argument!=nullptr){
						ubaciSadrzajUSekc(argument->name,locationCounter);
						locationCounter=locationCounter+4;
						argument=argument->next;
					}
					break;
			}
		}
			
		else if(strcmp(obrInstName,".section")==0){
      SectionDefinition* sd=nullptr;
				switch(i){
					case 1: // u prvom prolazu numerisu se samo sekcije, u drugom numerisemo sve simbole
						if (curSect!=nullptr) {
							for(SectionDefinition* sekcija:SEKCIJE){
								if (strcmp(curSect,sekcija->name)==0){
									sekcija->length=locationCounter; 	// minus base koji je 0 uvek
								}
							}
						}
						 
						curSect=obr_inst->args->name;
						locationCounter=0;
						poolCounter=0;
						
						sd = new SectionDefinition();
						sd->base=0;
						sd->length=0;
						sd->poolLength=0;
						sd->name=curSect;
						SEKCIJE.push_back(sd);				
						// i dodati novu sekciju u tabelu simbola
						dodUTabSimb_SEKCIJA(obr_inst->args->name,locationCounter);
						break;
					case 2:
						// otvaramo novu sekciju, i u staru upisujemo poolCounter koji se broji u drugom prolazu
						if (curSect!=nullptr) {
							for(SectionDefinition* sekcija:SEKCIJE){
								if (strcmp(curSect,sekcija->name)==0){
									sekcija->poolLength=poolCounter; 	
								}
							}
						}				
						curSect=obr_inst->args->name;
						poolCounter=0;
						locationCounter=0;
						break;
				}
		}
		
		else if(strcmp(obrInstName,".global")==0){
      bool vec_postoji=false;
				switch(i){
					case 1:												
						argument=obr_inst->args;
						while (argument!=nullptr){
							for(TabSimb* simbol:TABELA_SIMBOLA){
								if(strcmp(simbol->name,argument->name)==0){
									vec_postoji=true;
									simbol->globalDef=true;
									simbol->bind=2;
									if((simbol->externSymb)==true){
										std::cerr << "simbol .global i .extern !!!" << std::endl;
										std::exit(EXIT_FAILURE);
									}
								}
							}
							
							if(!vec_postoji){							
								TabSimb* simbol = new TabSimb();
								//simbol.num=++tabSimbNumeracija;  U DRUGOM PROLAZU
								simbol->value=0;
								simbol->type=1;
								simbol->bind=2; 
								simbol->globalDef=true;
								simbol->ndx=0;
								simbol->name=argument->name;								
								TABELA_SIMBOLA.push_back(simbol);							
							}
							
							vec_postoji=false;
							argument=argument->next;
						}
						break;
					case 2:					
						argument=obr_inst->args;
						while (argument!=nullptr){
							for(TabSimb* simbol:TABELA_SIMBOLA){
								if(strcmp(simbol->name,argument->name)==0){
									if((simbol->externSymb)==true || (simbol->ndx)<1 || (simbol->bind)==1){
										std::cerr << "simbol .global i jos nesto !!!" << std::endl;
										std::exit(EXIT_FAILURE);
									}
								}
							}						
							argument=argument->next;
						}						
						break;
				}
		}
		
		else if(strcmp(obrInstName,".extern")==0){
      bool vec_postoji=false;
				switch(i){
					case 1:						
						argument=obr_inst->args;
						while (argument!=nullptr){
							for(TabSimb* simbol:TABELA_SIMBOLA){
								if(strcmp(simbol->name,argument->name)==0){
									vec_postoji=true;
									simbol->externSymb=true;
									simbol->bind=2;
									if((simbol->globalDef)==true  || (simbol->ndx)>0){
										std::cerr << "simbol .global/ndx>0 i .extern !!!" << std::endl;
										std::exit(EXIT_FAILURE);
									}
								}
							}
							
							if(!vec_postoji){							
								TabSimb* simbol = new TabSimb();
								//simbol.num=++tabSimbNumeracija;  U DRUGOM PROLAZU
								simbol->value=0;
								simbol->type=1;
								simbol->bind=2; 
								simbol->externSymb=true;
								simbol->ndx=0;
								simbol->name=argument->name;								
								TABELA_SIMBOLA.push_back(simbol);							
							}
							
							vec_postoji=false;
							argument=argument->next;
						}
						break;
					case 2:
						argument=obr_inst->args;
						while (argument!=nullptr){
							for(TabSimb* simbol:TABELA_SIMBOLA){
								if(strcmp(simbol->name,argument->name)==0){
									if((simbol->globalDef)==true || (simbol->ndx)>0 || (simbol->bind)==1){
										std::cerr << "simbol .extern i jos nesto !!!" << std::endl;
										std::exit(EXIT_FAILURE);
									}
								}
							}						
							argument=argument->next;
						}
						break;
				}
		}
		
		else if(strcmp(obrInstName,".skip")==0){
      unsigned int broj = std::strtoul(obr_inst->args->name, nullptr, 0);
				switch(i){	
					case 1: // moze imati labelu
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+broj;
						break;
					case 2:	
						//numerisiIndxSIMBOL(obr_inst->label);					
						for (unsigned int i = 0; i < broj; i++) {
							ubaciSadrzajUSekc_BYTE(static_cast<unsigned char>(0x00),locationCounter);
							locationCounter=locationCounter+1;
						}
						break;
				}
		}
		
		else if(strcmp(obrInstName,"halt")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:						
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::HALT),"%r0","%r0","%r0",0,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
		
		else if(strcmp(obrInstName,"int")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::INT),"%r0","%r0","%r0",0,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"iret")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+12;
						break;
					case 2:
						//upisiInstrukciju(); PRVA
						upisiInstrukciju(static_cast<unsigned char>(INSTR::IRET1),"%sp","%sp","%r0",8,locationCounter);
						locationCounter=locationCounter+4;
						// DRUGA
						upisiInstrukciju(static_cast<unsigned char>(INSTR::IRET2),"%status","%sp","%r0",-4,locationCounter);
						locationCounter=locationCounter+4;
						// TRECA
						upisiInstrukciju(static_cast<unsigned char>(INSTR::IRET3),"%pc","%sp","%r0",-8,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"call")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						SectionDefinition* sekc;
						sekc=pronadjiSekciju(curSect);
						upisiInstrukciju(static_cast<unsigned char>(INSTR::CALL),"%pc","%r0","%r0",sekc->length-locationCounter-4+poolCounter,locationCounter);						
						// upisi u sect.length+poolCounter literal ili simbol(0x00000000+relok zapis)
						ubaciSadrzajUSekc(obr_inst->args->name,sekc->length+poolCounter);
						
						locationCounter=locationCounter+4;
						poolCounter=poolCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"ret")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::RET),"%pc","%sp","%r0",4,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"jmp")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						SectionDefinition* sekc;
						sekc=pronadjiSekciju(curSect);
						upisiInstrukciju(static_cast<unsigned char>(INSTR::JMP),"%pc","%r0","%r0",sekc->length-locationCounter-4+poolCounter,locationCounter);						
						// upisi u sect.length+poolCounter literal ili simbol(0x00000000+relok zapis)
						ubaciSadrzajUSekc(obr_inst->args->name,sekc->length+poolCounter);
						
						locationCounter=locationCounter+4;
						poolCounter=poolCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"beq")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						SectionDefinition* sekc;
						sekc=pronadjiSekciju(curSect);
						upisiInstrukciju(static_cast<unsigned char>(INSTR::BEQ),"%pc",obr_inst->args->name,obr_inst->args->next->name,sekc->length-locationCounter-4+poolCounter,locationCounter);						
						// upisi u sect.length+poolCounter literal ili simbol(0x00000000+relok zapis)
						ubaciSadrzajUSekc(obr_inst->args->next->next->name,sekc->length+poolCounter);
						
						locationCounter=locationCounter+4;
						poolCounter=poolCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"bne")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						SectionDefinition* sekc;
						sekc=pronadjiSekciju(curSect);
						upisiInstrukciju(static_cast<unsigned char>(INSTR::BNE),"%pc",obr_inst->args->name,obr_inst->args->next->name,sekc->length-locationCounter-4+poolCounter,locationCounter);						
						// upisi u sect.length+poolCounter literal ili simbol(0x00000000+relok zapis)
						ubaciSadrzajUSekc(obr_inst->args->next->next->name,sekc->length+poolCounter);
						
						locationCounter=locationCounter+4;
						poolCounter=poolCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"bgt")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						SectionDefinition* sekc;
						sekc=pronadjiSekciju(curSect);
						upisiInstrukciju(static_cast<unsigned char>(INSTR::BGT),"%pc",obr_inst->args->name,obr_inst->args->next->name,sekc->length-locationCounter-4+poolCounter,locationCounter);						
						// upisi u sect.length+poolCounter literal ili simbol(0x00000000+relok zapis)
						ubaciSadrzajUSekc(obr_inst->args->next->next->name,sekc->length+poolCounter);
						
						locationCounter=locationCounter+4;
						poolCounter=poolCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"push")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::PUSH),"%sp","%r0",obr_inst->args->name,-4,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"pop")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::POP),obr_inst->args->name,"%sp","%r0",4,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"xchg")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::XCHG),"%r0",obr_inst->args->name,obr_inst->args->next->name,0,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"add")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::ADD),obr_inst->args->next->name,obr_inst->args->next->name,obr_inst->args->name,0,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"sub")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::SUB),obr_inst->args->next->name,obr_inst->args->next->name,obr_inst->args->name,0,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"mul")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::MUL),obr_inst->args->next->name,obr_inst->args->next->name,obr_inst->args->name,0,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"div")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::DIV),obr_inst->args->next->name,obr_inst->args->next->name,obr_inst->args->name,0,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"not")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::NOT),obr_inst->args->name,obr_inst->args->name,"%r0",0,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"and")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::AND),obr_inst->args->next->name,obr_inst->args->next->name,obr_inst->args->name,0,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"or")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::OR),obr_inst->args->next->name,obr_inst->args->next->name,obr_inst->args->name,0,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"xor")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::XOR),obr_inst->args->next->name,obr_inst->args->next->name,obr_inst->args->name,0,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"shl")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::SHL),obr_inst->args->next->name,obr_inst->args->next->name,obr_inst->args->name,0,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"shr")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::SHR),obr_inst->args->next->name,obr_inst->args->next->name,obr_inst->args->name,0,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"ld")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						if(obr_inst->args->tip_arg==3 || obr_inst->args->tip_arg==4) locationCounter=locationCounter+8;
							else locationCounter=locationCounter+4;						
						break;
					case 2:
						//upisiInstrukciju();
						SectionDefinition* sekc;
						sekc=pronadjiSekciju(curSect);
							
						if(obr_inst->args->tip_arg==1 || obr_inst->args->tip_arg==2){						
							upisiInstrukciju(static_cast<unsigned char>((INSTR::LD)|0x02),obr_inst->args->next->name,"%pc","%r0",sekc->length-locationCounter-4+poolCounter,locationCounter);						
							// upisi u sect.length+poolCounter literal ili simbol(0x00000000+relok zapis)
							size_t duzin = std::strlen(obr_inst->args->name) - 1;
							char* b = new char[duzin + 1];
							std::strcpy(b, obr_inst->args->name + 1);
							POMOCNI_NIZOVI_CHAROVA.push_back(b);		// da bi smo dealocirali TEK KASNIJE
							ubaciSadrzajUSekc(b,sekc->length+poolCounter);
							poolCounter=poolCounter+4;							
						}else
						
						if(obr_inst->args->tip_arg==3 || obr_inst->args->tip_arg==4){
							upisiInstrukciju(static_cast<unsigned char>((INSTR::LD)|0x02),obr_inst->args->next->name,"%pc","%r0",sekc->length-locationCounter-4+poolCounter,locationCounter);
							ubaciSadrzajUSekc(obr_inst->args->name,sekc->length+poolCounter);
							poolCounter=poolCounter+4;
							locationCounter=locationCounter+4;
							upisiInstrukciju(static_cast<unsigned char>((INSTR::LD)|0x02),obr_inst->args->next->name,obr_inst->args->next->name,"%r0",0,locationCounter);
						}else
						
						if(obr_inst->args->tip_arg==5){
							upisiInstrukciju(static_cast<unsigned char>((INSTR::LD)|0x01),obr_inst->args->next->name,obr_inst->args->name,"%r0",0,locationCounter);
						}else
							
						if(obr_inst->args->tip_arg==6){
							upisiInstrukciju(static_cast<unsigned char>((INSTR::LD)|0x02),obr_inst->args->next->name,obr_inst->args->name,"%r0",0,locationCounter);
						}else
							
						if(obr_inst->args->tip_arg==7){
							int broj = std::strtoul(obr_inst->args->next->name, nullptr, 0);
							if (broj>2047 || broj<-2048) { std::cerr << "pomeraj veci od 12 bita" << std::endl; std::exit(EXIT_FAILURE);}
							
							upisiInstrukciju(static_cast<unsigned char>((INSTR::LD)|0x02),obr_inst->args->next->next->name,obr_inst->args->name,"%r0",static_cast<short>(broj),locationCounter);
						}
																		
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"st")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						SectionDefinition* sekc;
						sekc=pronadjiSekciju(curSect);
							
						if(obr_inst->args->next->tip_arg==3 || obr_inst->args->next->tip_arg==4){
							upisiInstrukciju(static_cast<unsigned char>((INSTR::ST)|0x02),"%pc","%r0",obr_inst->args->name,sekc->length-locationCounter-4+poolCounter,locationCounter);
							ubaciSadrzajUSekc(obr_inst->args->next->name,sekc->length+poolCounter);
							poolCounter=poolCounter+4;
						}else
							
						if(obr_inst->args->next->tip_arg==6){
							upisiInstrukciju(static_cast<unsigned char>((INSTR::ST)|0x00),obr_inst->args->next->name,"%r0",obr_inst->args->name,0,locationCounter);
						}else
							
						if(obr_inst->args->next->tip_arg==7){
							int broj = std::strtoul(obr_inst->args->next->next->name, nullptr, 0);
							if (broj>2047 || broj<-2048) { std::cerr << "pomeraj veci od 12 bita" << std::endl; std::exit(EXIT_FAILURE);}
							
							upisiInstrukciju(static_cast<unsigned char>((INSTR::ST)|0x00),obr_inst->args->next->name,"%r0",obr_inst->args->name,static_cast<short>(broj),locationCounter);
						}
																		
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"csrrd")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::CSRRD),obr_inst->args->next->name,obr_inst->args->name,"%r0",0,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
		else if(strcmp(obrInstName,"csrwr")==0){
				switch(i){
					case 1:
						if(obr_inst->label!=nullptr) dodUTabSimb_SIMBOL(obr_inst->label,locationCounter);
						locationCounter=locationCounter+4;
						break;
					case 2:
						//upisiInstrukciju();
						upisiInstrukciju(static_cast<unsigned char>(INSTR::CSRWR),obr_inst->args->next->name,obr_inst->args->name,"%r0",0,locationCounter);
						locationCounter=locationCounter+4;
						break;
				}
		}
			
			
		else{ std::cerr << "Nepoznata instrukcija" << std::endl; }
		
		
		
		obr_inst=obr_inst->next;
	}
	
	if (i==1){ // nakon svih instrukc i direkt, u prvom prolazu, zatvoriti poslednju sekciju odnosno update
		if (curSect!=nullptr) {
			for(SectionDefinition* sekcija:SEKCIJE){
				if (strcmp(curSect,sekcija->name)==0){
					sekcija->length=locationCounter; 	// minus base koji je 0 uvek
				}
			}
		}
	}
	if (i==2){ // za sekciju poolCounter update: poslednja kada se zatvara
		if (curSect!=nullptr) {
			for(SectionDefinition* sekcija:SEKCIJE){
				if (strcmp(curSect,sekcija->name)==0){
					sekcija->poolLength=poolCounter; 	
				}
			}
		}						
	}
	if (i==2){ // na kraju drugog prolaza da jos podesim num iz tabele simbola za sve simbole osim sekcija
		for(TabSimb* simbol : TABELA_SIMBOLA){
			if(simbol->bind==1 && simbol->type==1) numerisiIndxSIMBOL(simbol->name);
		}
		for(TabSimb* simbol : TABELA_SIMBOLA){
			if(simbol->bind==2 && simbol->type==1 && simbol->globalDef==1) numerisiIndxSIMBOL(simbol->name);
		}
    for(TabSimb* simbol : TABELA_SIMBOLA){
			if(simbol->bind==2 && simbol->type==1 && simbol->externSymb==1) numerisiIndxSIMBOL(simbol->name);
		}
	}
	
}





  //*********************************************************************************************************************************
  TABELA_SIMBOLA.sort([](TabSimb* a, TabSimb* b) {
        return a->num < b->num;
    });

  SADRZAJ_SEKCIJA.sort([](SadrSekc* a, SadrSekc* b) {
        return a->adr < b->adr;
    });
  
  RELOKACIONI_ZAPISI.sort([](RelZap* a, RelZap* b) {
        return a->offset < b->offset;
    });
// TABELA SIMBOLA
  std::cout << std::endl;std::cout << "TABELA SIMBOLA:";std::cout << std::endl;
  std::cout << "num   value     type   bind   ndx    globdef   extsimb      name  ";std::cout << std::endl;
  for(TabSimb *simbol : TABELA_SIMBOLA){
	  std::cout<<simbol->num<<"      ";
    std::cout<< simbol->value<<"       ";
    std::cout<< simbol->type<<"       ";
    std::cout<< simbol->bind<<"       ";
    std::cout<< simbol->ndx<<"       ";
    std::cout<< simbol->globalDef<<"          ";
    std::cout<< simbol->externSymb<<"    ";
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


  // KREIRANJE IZLAZA ASEMBLERA
  //*********************************************************************************************************************************

  // kreiraj izlaz iz asemblera
  std::ofstream binarniFajl(argv[2], std::ios::binary);
  if (!binarniFajl) {
        std::cerr << "Greška prilikom otvaranja fajla!" << std::endl;
        return 1;
    }

  // OVDE KUCAM
  //char chars[] = {'E', 'L', 'F', '2'};        // magicni broj
  //binarniFajl.write(chars, sizeof(chars));

  unsigned int brojElemenata = static_cast<unsigned int>(TABELA_SIMBOLA.size());          // broj simbola iz tabele simbola
  binarniFajl.write(reinterpret_cast<const char*>(&brojElemenata), sizeof(brojElemenata));

  for (TabSimb* simbol : TABELA_SIMBOLA) {                                                // tabela simbola + velicina imena simbola pre samog imena simbola
        binarniFajl.write(reinterpret_cast<const char*>(&simbol->num), sizeof(simbol->num));
        binarniFajl.write(reinterpret_cast<const char*>(&simbol->value), sizeof(simbol->value));
        binarniFajl.write(reinterpret_cast<const char*>(&simbol->type), sizeof(simbol->type));
        binarniFajl.write(reinterpret_cast<const char*>(&simbol->bind), sizeof(simbol->bind));
        binarniFajl.write(reinterpret_cast<const char*>(&simbol->ndx), sizeof(simbol->ndx));

        // Upisivanje veličine niza karaktera 'name'
        unsigned int velicinaImena = static_cast<unsigned int>(strlen(simbol->name));
        binarniFajl.write(reinterpret_cast<const char*>(&velicinaImena), sizeof(velicinaImena));

        // Upisivanje samog niza karaktera 'name'
        binarniFajl.write(simbol->name, velicinaImena);
    }

  unsigned int brojSekcija= static_cast<unsigned int>(SEKCIJE.size());          // broj sekcijaaa
  binarniFajl.write(reinterpret_cast<const char*>(&brojSekcija), sizeof(brojSekcija));


  for (SectionDefinition* sekcija : SEKCIJE) {                            // ZA SVAKU SEKCIJU
    unsigned int velicinaSekcije=sekcija->length+sekcija->poolLength;
    char* pomoc=sekcija->name;
    unsigned int brojRelokacZapisa = std::count_if(RELOKACIONI_ZAPISI.begin(), RELOKACIONI_ZAPISI.end(),
        [pomoc](const RelZap* relZapis) {
            return std::strcmp(relZapis->sekcija, pomoc) == 0;
        });
    
    binarniFajl.write(reinterpret_cast<const char*>(&velicinaSekcije), sizeof(velicinaSekcije));      // velicina sekcije u BAJTOVIMA
    binarniFajl.write(reinterpret_cast<const char*>(&brojRelokacZapisa), sizeof(brojRelokacZapisa));  // broj relokacionih zapisa

    unsigned int velicinaImena = static_cast<unsigned int>(strlen(sekcija->name));                    
    binarniFajl.write(reinterpret_cast<const char*>(&velicinaImena), sizeof(velicinaImena));        // velicina imena sekcije

    binarniFajl.write(sekcija->name, velicinaImena);                                                // ime sekcije
                                                                                                    // prvo idu svi bajtovi sekcije
                                                                                                    // pa onda idu svi relokacioni zapisi za sekciju

    for (SadrSekc* sadrzaj : SADRZAJ_SEKCIJA){                                                        // ovo je upis sadrzaja sekcije
      if(strcmp(sekcija->name,sadrzaj->sekcija)==0){
        binarniFajl.write(reinterpret_cast<const char*>(&sadrzaj->sadrzaj), sizeof(sadrzaj->sadrzaj));
      }
    }

    for (RelZap* relokZapis : RELOKACIONI_ZAPISI){                                                    // upis svih relok zapisa za trenutnu sekciju
      if(strcmp(sekcija->name,relokZapis->sekcija)==0){

        binarniFajl.write(reinterpret_cast<const char*>(&relokZapis->offset), sizeof(relokZapis->offset)); 
        binarniFajl.write(reinterpret_cast<const char*>(&relokZapis->type), sizeof(relokZapis->type)); 

        unsigned int velicinaImenaRelZap = static_cast<unsigned int>(strlen(relokZapis->simbol));                    
        binarniFajl.write(reinterpret_cast<const char*>(&velicinaImenaRelZap), sizeof(velicinaImenaRelZap));        // velicina imena rel zap

        binarniFajl.write(relokZapis->simbol, velicinaImenaRelZap);                                                 // ime rel zap ODNOSNO ime simbola

        binarniFajl.write(reinterpret_cast<const char*>(&relokZapis->addend), sizeof(relokZapis->addend)); 
      }

    }                                                                                           



  }






  binarniFajl.close();





  // OSLOBADJANJE MEMORIJE
  //*********************************************************************************************************************************
  for(char* naziv:POMOCNI_NIZOVI_CHAROVA){
    delete [] naziv;
  }
  for(TabSimb* simbol : TABELA_SIMBOLA){
    delete simbol;
  }
   for(RelZap* simbol : RELOKACIONI_ZAPISI){
    delete simbol;
  }
   for(SadrSekc* simbol : SADRZAJ_SEKCIJA){
    delete simbol;
  }
   for(SectionDefinition* simbol : SEKCIJE){
    delete simbol;
  }
  free_instrs(global_instr);
  
  fclose(myfile);

  

  std::cerr.rdbuf(cerr_buffer);
  return 0;
}