/**
 * @file      mips1_isa.cpp
 * @author    Sandro Rigo
 *            Marcus Bartholomeu
 *            Alexandro Baldassin (acasm information)
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:50:52 -0300
 * 
 * @brief     The ArchC i8051 functional model.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include  "mips1_isa.H"
#include  "mips1_isa_init.cpp"
#include  "mips1_bhv_macros.H"


//If you want debug information for this model, uncomment next line
//#define DEBUG_MODEL
#include "ac_debug_model.H"


//!User defined macros to reference registers.
#define Ra 31
#define Sp 29

// 'using namespace' statement to allow access to all
// mips1-specific datatypes
using namespace mips1_parms;


//!Generic instruction behavior method.
void ac_behavior( instruction )
{ 
  dbg_printf("----- PC=%#x ----- %lld\n", (int) ac_pc, ac_instr_counter);
switch( stage ) { 
	case IF: 
		ac_pc += 4; 
		IF_ID.npc = ac_pc; 
		break;
	case ID:
		ID_EX.npc.write(IF_ID.npc.read());
		ID_EX.rs = rs;
		ID_EX.rt = rt;
		ID_EX.imm = imm;
		break;
	case EX:
		EX_MEM.memread.write(ID_EX.memread.read());
		EX_MEM.regread.write(ID_EX.regread.read());
		EX_MEM.regwrite.write(ID_EX.regwrite.read());
		break;
	case MEM:
		MEM_WB.regwrite.write(EX_MEM.regread.read());
		if((EX_MEM.memread == 1) && (EX_MEM.regwrite == 1))MEM_WB.wbdata.write(DM.read(EX_MEM.alures.read()));
		break;
	case WB:		
		// Execute write back when allowed 
		 if (MEM_WB.regwrite == 1) {
		// Register 0 is never written 
		if (MEM_WB.rdest != 0) RB.write(MEM_WB.rdest.read(), MEM_WB.wbdata.read());
		}
		break;
	default: 
		break; 
} 
};
 
//! Instruction Format behavior methods.
void ac_behavior( Type_R )
{switch( stage ) {
		case ID:
				ID_EX.data1.write(RB[rs]);
				ID_EX.data2.write(RB[rt]);
					break;
		case EX:
			EX_MEM.rdest = ID_EX.rd;
		// Checking forwarding for the rs register 
			if ( (EX_MEM.regwrite == 1) && (EX_MEM.rdest != 0) && (EX_MEM.rdest == ID_EX.rs) ){
				operand1 = EX_MEM.alures.read(); 
				}else if ( (MEM_WB.regwrite == 1) && (MEM_WB.rdest != 0) && 
					(MEM_WB.rdest == ID_EX.rs) && (EX_MEM.rdest == ID_EX.rs) ){
					operand1 = MEM_WB.wbdata.read(); 
				}else operand1 = ID_EX.data1.read();
			// Checking forwarding for the rt register  
			if ( (EX_MEM.regwrite == 1) && (EX_MEM.rdest != 0) && (EX_MEM.rdest == ID_EX.rt) ){
				operand2 = EX_MEM.alures.read(); 
				}else if ( (MEM_WB.regwrite == 1) && (MEM_WB.rdest != 0) && (MEM_WB.rdest == ID_EX.rt) && 
					(EX_MEM.rdest == ID_EX.rt) ){
					operand2 = MEM_WB.wbdata.read(); 
				}else{ 
					operand2 = ID_EX.data2.read();
				}
  
	break;
default: 
	break; 
}
}
void ac_behavior( Type_I )
{switch( stage ) {
		case ID:
			ID_EX.data1.write(RB[rs]);
			ID_EX.data2.write(RB[rt]);
				break;
		case EX:
			EX_MEM.rdest = ID_EX.rt;
				break;
		default:
			break;
			}
}
void ac_behavior( Type_J ){}
 
//!Behavior called before starting simulation
void ac_behavior(begin)
{
  dbg_printf("@@@ begin behavior @@@\n");
  RB[0] = 0;
  npc = ac_pc + 4;

  // Is is not required by the architecture, but makes debug really easier
  for (int regNum = 0; regNum < 32; regNum ++)
    RB[regNum] = 0;
  hi = 0;
  lo = 0;
}

//!Behavior called after finishing simulation
void ac_behavior(end)
{
  dbg_printf("@@@ end behavior @@@\n");
}


//!Instruction lb behavior method.
void ac_behavior( lb, stage )
{switch(stage) { 
		case IF: 
			break;
		case ID: 
			ID_EX.regwrite.write(1);
			ID_EX.memwrite.write(0);
			ID_EX.memread.write(1);
			break;
		case EX:			
			EX_MEM.alures.write(ID_EX.data1.read() + ID_EX.imm.read());	
			break; 
		case MEM: 
			if(ID_EX.memread == 1){
			char byte;
			byte = DM.read_byte(EX_MEM.alures.read());
			MEM_WB.wbdata.write((ac_Sword)byte);
			}
			break;
		case WB:
			break;
		default:
			 break; 
	}
 /* char byte;
  dbg_printf("lb r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  MEM_WB.wbdata.write(DM.read_byte(EX_MEM.alures.read()));
  RB[rt] = (ac_Sword)MEM_WB.wbdata.read() ;
  dbg_printf("Result = %#x\n", RB[rt]);
*/
};

//!Instruction lbu behavior method.
void ac_behavior( lbu, stage )
{switch(stage) {
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(1);
                        break;
                case EX:
					EX_MEM.alures.write(ID_EX.data1.read() + ID_EX.imm.read());	
                        break;
                case MEM:
					if(ID_EX.memread == 1){
					unsigned char byte;
					byte = DM.read_byte(EX_MEM.alures.read());
					MEM_WB.wbdata.write(byte);
					}
                        break;
                case WB:
                     break;
                default:
                         break;
        }

  /*unsigned char byte;
  dbg_printf("lbu r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  byte = DM.read_byte(RB[rs]+ imm);
  RB[rt] = byte ;
  dbg_printf("Result = %#x\n", RB[rt]);*/
};

//!Instruction lh behavior method.
void ac_behavior( lh, stage )
{switch(stage) {
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(1);
                        break;
                case EX:
					EX_MEM.alures.write(ID_EX.data1.read() + ID_EX.imm.read());	
                        break;
                case MEM:
					if(ID_EX.memread == 1){
					short int half;
					half = DM.read_half(EX_MEM.alures.read());
					MEM_WB.wbdata.write((ac_Sword)half);
					}
                        break;
                case WB:
                     break;
                default:
                         break;
        }
 /* short int half;
  dbg_printf("lh r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  half = DM.read_half(RB[rs]+ imm);
  RB[rt] = (ac_Sword)half ;
  dbg_printf("Result = %#x\n", RB[rt]);*/
};

//!Instruction lhu behavior method.
void ac_behavior( lhu, stage )
{switch(stage) {
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(1);
                        break;
                case EX:
					EX_MEM.alures.write(ID_EX.data1.read() + ID_EX.imm.read());	
                        break;
                case MEM:
					if(ID_EX.memread == 1){
					unsigned short int  half;
					half = DM.read_half(EX_MEM.alures.read());
					MEM_WB.wbdata.write(half);
					}
                        break;
                case WB:
                     break;
                default:
                         break;
        }
		/*
  unsigned short int  half;
  half = DM.read_half(RB[rs]+ imm);
  RB[rt] = half ;
  dbg_printf("Result = %#x\n", RB[rt]);*/
};

//!Instruction lw behavior method.
void ac_behavior( lw, stage )
{switch(stage) {
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(1);
                        break;
                case EX:
					EX_MEM.alures.write(ID_EX.data1.read() + ID_EX.imm.read());	
                        break;
                case MEM:
					if(ID_EX.memread == 1){
					MEM_WB.wbdata.write(DM.read(EX_MEM.alures.read()));
					}
                        break;
                case WB:
                       break;
                default:
                         break;
        }
		/*
  dbg_printf("lw r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  RB[rt] = DM.read(RB[rs]+ imm);
  dbg_printf("Result = %#x\n", RB[rt]);*/
};

//!Instruction lwl behavior method.
void ac_behavior( lwl, stage )
{switch(stage) {
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(1);
                        break;
                case EX:
					EX_MEM.alures.write(ID_EX.data1.read() + ID_EX.imm.read());	
                        break;
                case MEM:
					if(ID_EX.memread == 1){
					unsigned int offset;
					ac_Uword data;
					data = DM.read(EX_MEM.alures.read()& 0xFFFFFFFC);
					offset = ((EX_MEM.alures.read() & 0x3) * 8);
					data <<= offset;
					data |= RB[EX_MEM.regdest] & ((1<<offset)-1); //nao posso acessar o banco de reg aqui!!
					MEM_WB.wbdata.write(data);
					}
                        break;
                case WB:
                        break;
                default:
                         break;
        }
		/*
  dbg_printf("lwl r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (addr & 0x3) * 8;
  data = DM.read(addr & 0xFFFFFFFC);
  data <<= offset;
  data |= RB[rt] & ((1<<offset)-1);
  RB[rt] = data;
  dbg_printf("Result = %#x\n", RB[rt]);*/
};

//!Instruction lwr behavior method.
void ac_behavior( lwr, stage )
{switch(stage) {
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(1);
                        break;
                case EX:
					EX_MEM.alures.write(ID_EX.data1.read() + ID_EX.imm.read());	
                        break;
                case MEM:
					if(ID_EX.memread == 1){
					unsigned int offset;
					ac_Uword data;
					data = DM.read(EX_MEM.alures.read()& 0xFFFFFFFC);
					offset = ((3 - (EX_MEM.alures.read() & 0x3)) * 8);
					data =>> offset;
					data |= RB[EX_MEM.regdest] & (0xFFFFFFFF << (32-offset));//nao posso acessar o banco de reg aqui!!
					MEM_WB.wbdata.write(data);
					}
                        break;
                case WB:
                        break;
                default:
                         break;
        }
		/*
  dbg_printf("lwr r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (3 - (addr & 0x3)) * 8;
  data = DM.read(addr & 0xFFFFFFFC);
  data >>= offset;
  data |= RB[rt] & (0xFFFFFFFF << (32-offset));
  RB[rt] = data;
  dbg_printf("Result = %#x\n", RB[rt]);*/
};

//!Instruction sb behavior method.
void ac_behavior( sb, stage )
{switch(stage) {
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(1);
                    ID_EX.memread.write(0);
                        break;
                case EX:
					EX_MEM.alures.write(ID_EX.data1.read() + ID_EX.imm.read());	
					unsigned char byte;
					byte = ID_EX.data2.read() & 0xFF;
					EX_MEM.wdata.write(byte);
                        break;
                case MEM:
					if(ID_EX.memwrite == 1){
					DM.write_byte(EX_MEM.alures.read(), EX_MEM.wdata.read());//precisa de cast?
					}
                        break;
                case WB:
                        break;
                default:
                         break;
        }
		/*
  unsigned char byte;
  dbg_printf("sb r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  byte = RB[rt] & 0xFF;
  DM.write_byte(RB[rs] + imm, byte);
  dbg_printf("Result = %#x\n", (int) byte);*/
};

//!Instruction sh behavior method.
void ac_behavior( sh, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(1);
                    ID_EX.memread.write(0);
                        break;
                case EX:
					EX_MEM.alures.write(ID_EX.data1.read() + ID_EX.imm.read());	
					unsigned short int half;
					half = ID_EX.data2.read() & 0xFFFF;
					EX_MEM.wdata.write(half);
                        break;
                case MEM:
					if(ID_EX.memwrite == 1){
					DM.write_half(EX_MEM.alures.read(), EX_MEM.wdata.read());//precisa de cast?
					}
                        break;
                case WB:
                        break;
                default:
                         break;
        }
  /*unsigned short int half;
  dbg_printf("sh r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  half = RB[rt] & 0xFFFF;
  DM.write_half(RB[rs] + imm, half);
  dbg_printf("Result = %#x\n", (int) half);*/
};

//!Instruction sw behavior method.
void ac_behavior( sw, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(1);
                    ID_EX.memread.write(0);
                        break;
                case EX:
					EX_MEM.alures.write(ID_EX.data1.read() + ID_EX.imm.read());	
					EX_MEM.wdata.write(ID_EX.data2.read());
                        break;
                case MEM:
					if(ID_EX.memwrite == 1){
					DM.write(EX_MEM.alures.read(), EX_MEM.wdata.read());
					}
                        break;
                case WB:
                        break;
                default:
                         break;
        }
		/*
  dbg_printf("sw r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  DM.write(RB[rs] + imm, RB[rt]);
  dbg_printf("Result = %#x\n", RB[rt]);*/
};

//!Instruction swl behavior method.
void ac_behavior( swl, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(1);
                    ID_EX.memread.write(1);
                        break;
                case EX:
					EX_MEM.alures.write(ID_EX.data1.read() + ID_EX.imm.read());	
					ac_Uword data;					
					data = ID_EX.data2.read();						
					EX_MEM.wdata.write(data);
                        break;
                case MEM:
					unsigned int offset;
					offset = (EX_MEM.alures.read() & 0x3) * 8;
					data >>= offset;
					if((ID_EX.memwrite == 1)&& ID_EX.memread == 1){					
						data |= DM.read(EX_MEM.alures.read() & 0xFFFFFFFC) & (0xFFFFFFFF << (32-offset));
						DM.write(EX_MEM.alures.read() & 0xFFFFFFFC, EX_MEM.wdata.read());
					}
                        break;
                case WB:
                        break;
                default:
       }         
 /* dbg_printf("swl r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (addr & 0x3) * 8;
  data = RB[rt];
  data >>= offset;
  data |= DM.read(addr & 0xFFFFFFFC) & (0xFFFFFFFF << (32-offset));
  DM.write(addr & 0xFFFFFFFC, data);
  dbg_printf("Result = %#x\n", data);*/
};

//!Instruction swr behavior method.
void ac_behavior( swr, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(1);
                    ID_EX.memread.write(1);
                        break;
                case EX:
					EX_MEM.alures.write(ID_EX.data1.read() + ID_EX.imm.read());	
					ac_Uword data;					
				    data = ID_EX.data2.read();						
					EX_MEM.wdata.write(data);
                        break;
                case MEM:
					unsigned int offset;
					offset = 3 - ((EX_MEM.alures.read() & 0x3)) * 8;
					data >>= offset;
					if((ID_EX.memwrite == 1)&& ID_EX.memread == 1){					
						data |= DM.read(EX_MEM.alures.read() & 0xFFFFFFFC) & ((1<<offset)-1);
						DM.write(EX_MEM.alures.read() & 0xFFFFFFFC, EX_MEM.wdata.read());
					}
                        break;
                case WB:
                        break;
                default:
       }         
	   /*
  dbg_printf("swr r%d, %d(r%d)\n", rt, imm & 0xFFFF, rs);
  unsigned int addr, offset;
  ac_Uword data;

  addr = RB[rs] + imm;
  offset = (3 - (addr & 0x3)) * 8;
  data = RB[rt];
  data <<= offset;
  data |= DM.read(addr & 0xFFFFFFFC) & ((1<<offset)-1);
  DM.write(addr & 0xFFFFFFFC, data);
   addr = RB[rs] + imm;dbg_printf("Result = %#x\n", data);
   */
};

//!Instruction addi behavior method.
void ac_behavior( addi, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:						
					EX_MEM.alures.write(RB[ID_EX.rs] + ID_EX.imm.read());	
					  break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }         
	   /*
  dbg_printf("addi r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] + imm;
  dbg_printf("Result = %#x\n", RB[rt]);
  //Test overflow
  if ( ((RB[rs] & 0x80000000) == (imm & 0x80000000)) &&
       ((imm & 0x80000000) != (RB[rt] & 0x80000000)) ) {
    fprintf(stderr, "EXCEPTION(addi): integer overflow.\n"); exit(EXIT_FAILURE);
  }
  */
};

//!Instruction addiu behavior method.
void ac_behavior( addiu, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(RB[ID_EX.rs] + ID_EX.imm.read());	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }      
	   /*
  dbg_printf("addiu r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] + imm;
  dbg_printf("Result = %#x\n", RB[rt]);
  */
};

//!Instruction slti behavior method.
void ac_behavior( slti, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:		
					if( (ac_Sword) ID_EX.rs < (ac_Sword) ID_EX.imm )
						EX_MEM.alures.write(1);
					// Else reset RD
					else
					EX_MEM.alures.write(0);
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }      
	   /*
  dbg_printf("slti r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  // Set the RD if RS< IMM
  if( (ac_Sword) RB[rs] < (ac_Sword) imm )
    RB[rt] = 1;
  // Else reset RD
  else
    RB[rt] = 0;
  dbg_printf("Result = %#x\n", RB[rt]);
  */
};

//!Instruction sltiu behavior method.
void ac_behavior( sltiu, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:		
					if( (ac_Uword) ID_EX.rs < (ac_Uword) ID_EX.imm )
						EX_MEM.alures.write(1);
					// Else reset RD
					else
						EX_MEM.alures.write(0);
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }     
	   /*
  dbg_printf("sltiu r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  // Set the RD if RS< IMM
  if( (ac_Uword) RB[rs] < (ac_Uword) imm )
    RB[rt] = 1;
  // Else reset RD
  else
    RB[rt] = 0;
  dbg_printf("Result = %#x\n", RB[rt]);*/
};

//!Instruction andi behavior method.
void ac_behavior( andi, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(ID_EX.data1.read() & (ID_EX.imm.read() & 0xFFFF));	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }     
/*	   
  dbg_printf("andi r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] & (imm & 0xFFFF) ;
  dbg_printf("Result = %#x\n", RB[rt]);
  */
};

//!Instruction ori behavior method.
void ac_behavior( ori, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(ID_EX.data1.read() | (ID_EX.imm.read() & 0xFFFF));	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }     
/*	   
  dbg_printf("ori r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] | (imm & 0xFFFF) ;
  dbg_printf("Result = %#x\n", RB[rt]);
*/
  };

//!Instruction xori behavior method.
void ac_behavior( xori, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(ID_EX.data1.read() ^ (ID_EX.imm.read() & 0xFFFF));	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }     
/*	   
  dbg_printf("xori r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  RB[rt] = RB[rs] ^ (imm & 0xFFFF) ;
  dbg_printf("Result = %#x\n", RB[rt]);*/
};

//!Instruction lui behavior method.
void ac_behavior( lui )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(ID_EX.imm.read() << 16));	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }     
/*	   
  dbg_printf("lui r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  // Load a constant in the upper 16 bits of a register
  // To achieve the desired behaviour, the constant was shifted 16 bits left
  // and moved to the target register ( rt )
  RB[rt] = imm << 16;
  dbg_printf("Result = %#x\n", RB[rt]);*/
};

//!Instruction add behavior method.
void ac_behavior( add, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(ID_EX.data1.read() + ID_EX.data2.read());	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }     
	   /*
  dbg_printf("add r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] + RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
  //Test overflow
  if ( ((RB[rs] & 0x80000000) == (RB[rd] & 0x80000000)) &&
       ((RB[rd] & 0x80000000) != (RB[rt] & 0x80000000)) ) {
    fprintf(stderr, "EXCEPTION(add): integer overflow.\n"); exit(EXIT_FAILURE);*/
  }
};

//!Instruction addu behavior method.
void ac_behavior( addu, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(ID_EX.data1.read() + ID_EX.data2.read());	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }     
	   /*
  dbg_printf("addu r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] + RB[rt];
  //cout << "  RS: " << (unsigned int)RB[rs] << " RT: " << (unsigned int)RB[rt] << endl;
  //cout << "  Result =  " <<  (unsigned int)RB[rd] <<endl;
  dbg_printf("Result = %#x\n", RB[rd]);*/
};

//!Instruction sub behavior method.
void ac_behavior( sub, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(ID_EX.data1.read() - ID_EX.data2.read());	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }     
	   /*
  dbg_printf("sub r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] - RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
  //TODO: test integer overflow exception for sub*/
};

//!Instruction subu behavior method.
void ac_behavior( subu, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(ID_EX.data1.read() - ID_EX.data2.read());	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	   
  dbg_printf("subu r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] - RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);
  */
};

//!Instruction slt behavior method.
void ac_behavior( slt, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					// Set the RD if RS< RT
					if( (ac_Sword) ID_EX.rs < (ac_Sword) ID_EX.rt )
						EX_MEM.rd.write(1);
					// Else reset RD
					else
						EX_MEM.rd.write(0);
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }     
/*	   
  dbg_printf("slt r%d, r%d, r%d\n", rd, rs, rt);
  // Set the RD if RS< RT
  if( (ac_Sword) RB[rs] < (ac_Sword) RB[rt] )
    RB[rd] = 1;
  // Else reset RD
  else
    RB[rd] = 0;
  dbg_printf("Result = %#x\n", RB[rd]);*/
};

//!Instruction sltu behavior method.
void ac_behavior( sltu, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					// Set the RD if RS< RT
					if(ID_EX.rs < ID_EX.rt )
						EX_MEM.rd.write(1);
					// Else reset RD
					else
						EX_MEM.rd.write(0);
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }     
/*	   
  dbg_printf("sltu r%d, r%d, r%d\n", rd, rs, rt);
  // Set the RD if RS < RT
  if( RB[rs] < RB[rt] )
    RB[rd] = 1;
  // Else reset RD
  else
    RB[rd] = 0;
  dbg_printf("Result = %#x\n", RB[rd]);*/
};

//!Instruction instr_and behavior method.
void ac_behavior( instr_and, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(ID_EX.data1.read() & ID_EX.data2.read());	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	   
  dbg_printf("instr_and r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] & RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);*/
};

//!Instruction instr_or behavior method.
void ac_behavior( instr_or, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(ID_EX.data1.read() | ID_EX.data2.read());	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	   
  dbg_printf("instr_or r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] | RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);*/
};

//!Instruction instr_xor behavior method.
void ac_behavior( instr_xor, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(ID_EX.data1.read() ^ ID_EX.data2.read());	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	   
  dbg_printf("instr_xor r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = RB[rs] ^ RB[rt];
  dbg_printf("Result = %#x\n", RB[rd]);*/
};

//!Instruction instr_nor behavior method.
void ac_behavior( instr_nor, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(~(ID_EX.data1.read() | ID_EX.data2.read()));	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	   
  dbg_printf("nor r%d, r%d, r%d\n", rd, rs, rt);
  RB[rd] = ~(RB[rs] | RB[rt]);
  dbg_printf("Result = %#x\n", RB[rd]);*/
};

//!Instruction nop behavior method.
void ac_behavior( nop, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:						
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	     
  dbg_printf("nop\n");*/
};

//!Instruction sll behavior method.
void ac_behavior( sll, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(ID_EX.data2.read() << shamt);	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	     
  dbg_printf("sll r%d, r%d, %d\n", rd, rs, shamt);
  RB[rd] = RB[rt] << shamt;
  dbg_printf("Result = %#x\n", RB[rd]);*/
};

//!Instruction srl behavior method.
void ac_behavior( srl, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(ID_EX.data2.read() >> shamt);	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	     
  dbg_printf("srl r%d, r%d, %d\n", rd, rs, shamt);
  RB[rd] = RB[rt] >> shamt;
  dbg_printf("Result = %#x\n", RB[rd]);*/
};

//!Instruction sra behavior method.
void ac_behavior( sra, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write((ac_Sword)ID_EX.data2.read() >> shamt);	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	     
  dbg_printf("sra r%d, r%d, %d\n", rd, rs, shamt);
  RB[rd] = (ac_Sword) RB[rt] >> shamt;
  dbg_printf("Result = %#x\n", RB[rd]);*/
};

//!Instruction sllv behavior method.
void ac_behavior( sllv, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write((ac_Sword)ID_EX.data2.read() << ID_EX.data1.read() & 0x1F) );	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	
  dbg_printf("sllv r%d, r%d, r%d\n", rd, rt, rs);
  RB[rd] = RB[rt] << (RB[rs] & 0x1F);
  dbg_printf("Result = %#x\n", RB[rd]);*/
};

//!Instruction srlv behavior method.
void ac_behavior( srlv, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write(ID_EX.data2.read() >> (ID_EX.data1.read() & 0x1F) );	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	
  dbg_printf("srlv r%d, r%d, r%d\n", rd, rt, rs);
  RB[rd] = RB[rt] >> (RB[rs] & 0x1F);
  dbg_printf("Result = %#x\n", RB[rd]);*/
};

//!Instruction srav behavior method.
void ac_behavior( srav, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					EX_MEM.alures.write((ac_Sword)ID_EX.data2.read() << (ID_EX.data1.read() & 0x1F) );	
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	
  dbg_printf("srav r%d, r%d, r%d\n", rd, rt, rs);
  RB[rd] = (ac_Sword) RB[rt] >> (RB[rs] & 0x1F);
  dbg_printf("Result = %#x\n", RB[rd]);*/
};

//!Instruction mult behavior method.
void ac_behavior( mult, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					long long result;
					int half_result;

					result = (ac_Sword) ID_EX.data1.read();
					result *= (ac_Sword) ID_EX.data2.read();

					half_result = (result & 0xFFFFFFFF);
					// Register LO receives 32 less significant bits
					lo = half_result;

					half_result = ((result >> 32) & 0xFFFFFFFF);
					// Register HI receives 32 most significant bits
					hi = half_result ;
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	
  dbg_printf("mult r%d, r%d\n", rs, rt);

  long long result;
  int half_result;

  result = (ac_Sword) RB[rs];
  result *= (ac_Sword) RB[rt];

  half_result = (result & 0xFFFFFFFF);
  // Register LO receives 32 less significant bits
  lo = half_result;

  half_result = ((result >> 32) & 0xFFFFFFFF);
  // Register HI receives 32 most significant bits
  hi = half_result ;

  dbg_printf("Result = %#llx\n", result);*/
};

//!Instruction multu behavior method.
void ac_behavior( multu, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
                        break;
                case EX:	
					unsigned long long result;
					unsigned int half_result;

					result = ID_EX.data1.read();
					result *= ID_EX.data2.read();

					half_result = (result & 0xFFFFFFFF);
					// Register LO receives 32 less significant bits
					lo = half_result;

					half_result = ((result >> 32) & 0xFFFFFFFF);
					// Register HI receives 32 most significant bits
					hi = half_result ;
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
	   /*
  dbg_printf("multu r%d, r%d\n", rs, rt);

  unsigned long long result;
  unsigned int half_result;

  result  = RB[rs];
  result *= RB[rt];

  half_result = (result & 0xFFFFFFFF);
  // Register LO receives 32 less significant bits
  lo = half_result;

  half_result = ((result>>32) & 0xFFFFFFFF);
  // Register HI receives 32 most significant bits
  hi = half_result ;

  dbg_printf("Result = %#llx\n", result);*/
};

//!Instruction div behavior method.
void ac_behavior( div, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);					
                        break;
                case EX:	
					// Register LO receives quotient
					lo = (ac_Sword) ID_EX.data1.read() / (ac_Sword) ID_EX.data2.read();;
					 // Register HI receives remainder
					hi = (ac_Sword) ID_EX.data1.read() % (ac_Sword) ID_EX.data2.read();
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	
  dbg_printf("div r%d, r%d\n", rs, rt);
  // Register LO receives quotient
  lo = (ac_Sword) RB[rs] / (ac_Sword) RB[rt];
  // Register HI receives remainder
  hi = (ac_Sword) RB[rs] % (ac_Sword) RB[rt];*/
};

//!Instruction divu behavior method.
void ac_behavior( divu, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
					
                        break;
                case EX:	
					// Register LO receives quotient
					lo =  ID_EX.data1.read() / ID_EX.data2.read();
					 // Register HI receives remainder
					hi = ID_EX.data1.read() % ID_EX.data2.read();
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	
  dbg_printf("divu r%d, r%d\n", rs, rt);
  // Register LO receives quotient
  lo = RB[rs] / RB[rt];
  // Register HI receives remainder
  hi = RB[rs] % RB[rt];*/
};

//!Instruction mfhi behavior method.
void ac_behavior( mfhi, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
					RB[rd] = hi;
                        break;
                case EX:						
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	
  dbg_printf("mfhi r%d\n", rd);
  RB[rd] = hi;
  dbg_printf("Result = %#x\n", RB[rd]);*/
};

//!Instruction mthi behavior method.
void ac_behavior( mthi, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);		
                        break;
                case EX:		
					hi = ID_EX.data1.read();
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	
  dbg_printf("mthi r%d\n", rs);
  hi = RB[rs];
  dbg_printf("Result = %#x\n", hi);*/
};

//!Instruction mflo behavior method.
void ac_behavior( mflo, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);
					RB[rd] = lo;
                        break;
                case EX:						
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	
  dbg_printf("mflo r%d\n", rd);
  RB[rd] = lo;
  dbg_printf("Result = %#x\n", RB[rd]);*/
};

//!Instruction mtlo behavior method.
void ac_behavior( mtlo, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);		
                        break;
                case EX:		
					hi = ID_EX.data1.read();
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	
  dbg_printf("mtlo r%d\n", rs);
  lo = RB[rs];
  dbg_printf("Result = %#x\n", lo);*/
};

//!Instruction j behavior method.
void ac_behavior( j )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);		
                        break;
                case EX:		
					addr = addr << 2;
					npc =  (ID_EX.npc & 0xF0000000) | addr;
				//	ac_flush(IF);
				//	ac_flush(ID);
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	
  dbg_printf("j %d\n", addr);
  addr = addr << 2;
#ifndef NO_NEED_PC_UPDATE
  npc =  (ac_pc & 0xF0000000) | addr;
#endif 
  dbg_printf("Target = %#x\n", (ac_pc & 0xF0000000) | addr );*/
};

//!Instruction jal behavior method.
void ac_behavior( jal, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);		
					
                        break;
                case EX:		
					addr = addr << 2;
					npc =  (ID_EX.npc & 0xF0000000) | addr;
					EX_MEM.alures.write(ID_EX.npc+4);	
					ac_flush(IF);
					ac_flush(ID);
                        break;
                case MEM:					
					MEM_WB.wbdata.write(EX_MEM.alures.read());
                        break;
                case WB:
				RB[Ra] = MEM_WB.wbdata.read()
                        break;
                default:
       }  
/*	
  dbg_printf("jal %d\n", addr);
  // Save the value of PC + 8 (return address) in $ra ($31) and
  // jump to the address given by PC(31...28)||(addr<<2)
  // It must also flush the instructions that were loaded into the pipeline
  RB[Ra] = ac_pc+4; //ac_pc is pc+4, we need pc+8
	
  addr = addr << 2;
#ifndef NO_NEED_PC_UPDATE
  npc = (ac_pc & 0xF0000000) | addr;
#endif 
	
  dbg_printf("Target = %#x\n", (ac_pc & 0xF0000000) | addr );
  dbg_printf("Return = %#x\n", ac_pc+4);*/
};

//!Instruction jr behavior method.
void ac_behavior( jr, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);		
					ID_EX.data1.write(RB[rs]);
                        break;
                case EX:		
					npc = ID_EX.data1.read(), 1;
					ac_flush(IF);
					ac_flush(ID);
                        break;
                case MEM:					
                        break;
                case WB:
                        break;
                default:
       }  
/*	
  dbg_printf("jr r%d\n", rs);
  // Jump to the address stored on the register reg[RS]
  // It must also flush the instructions that were loaded into the pipeline
#ifndef NO_NEED_PC_UPDATE
  npc = RB[rs], 1;
#endif 
  dbg_printf("Target = %#x\n", RB[rs]);*/
};

//!Instruction jalr behavior method.
void ac_behavior( jalr, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);		
					ID_EX.data1.write(RB[rs]);
					if( rd == 0 )  //If rd is not defined use default
						rd = Ra;
					EX_MEM.alures.write(ID_EX.npc+4);	
                        break;
                case EX:		
					npc = ID_EX.data1.read(), 1;	//como que guardo duas op de alu???
					EX_MEM.alures.write(ID_EX.npc+4);					
				//	ac_flush(IF);
				//	ac_flush(ID);
                        break;
                case MEM:		
					MEM_WB.wbdata.write(EX_MEM.alures.read());				
                        break;
                case WB:
                        break;
                default:
       }  
/*	
  dbg_printf("jalr r%d, r%d\n", rd, rs);
  // Save the value of PC + 8(return address) in rd and
  // jump to the address given by [rs]

#ifndef NO_NEED_PC_UPDATE
  npc = RB[rs], 1;
#endif 
  dbg_printf("Target = %#x\n", RB[rs]);

  if( rd == 0 )  //If rd is not defined use default
    rd = Ra;
  RB[rd] = ac_pc+4;
  dbg_printf("Return = %#x\n", ac_pc+4);*/
};

//!Instruction beq behavior method.
void ac_behavior( beq, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(1);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);		
					ID_EX.data1.write(RB[rs]);
					ID_EX.data2.write(RB[rt]);					
                        break;
                case EX:		
					if( ID_EX.data1.read() == ID_EX.data2.read() ){
						EX_MEM.alures.write(ID_EX.npc + (ID_EX.imm<<2));
					}else{
						EX_MEM.alures.write(ID_EX.npc);
						}
                        break;
                case MEM:	
					npc = EX_MEM.alures.read();
                        break;
                case WB:
                        break;
                default:
       }  
/*	
  dbg_printf("beq r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  if( RB[rs] == RB[rt] ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }	*/
};

//!Instruction bne behavior method.
void ac_behavior( bne, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);		
					ID_EX.data1.write(RB[rs]);
					ID_EX.data2.write(RB[rt]);								
                        break;
                case EX:		
					if( ID_EX.data1.read() != ID_EX.data2.read() ){
						EX_MEM.alures.write(ID_EX.npc.read() + (imm<<2));
					}else{
						EX_MEM.alures.write(ID_EX.npc);
						}						
					//ac_flush(IF);
					//ac_flush(ID);
                        break;
                case MEM:		
					npc = EX_MEM.alures.read();
                        break;
                case WB:
                        break;
                default:
       }  
/*		
  dbg_printf("bne r%d, r%d, %d\n", rt, rs, imm & 0xFFFF);
  if( RB[rs] != RB[rt] ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }	*/
};

//!Instruction blez behavior method.
void ac_behavior( blez, stage )
{switch(stage){
                case IF:
                        break;
                case ID:
                    ID_EX.regwrite.write(0);
                    ID_EX.memwrite.write(0);
                    ID_EX.memread.write(0);		
					ID_EX.data1.write(RB[rs]);									
                        break;
                case EX:		
					if( (ID_EX.data1.read() == 0 ) || (ID_EX.data1&0x80000000 ) ){
						EX_MEM.alures.write(ac_pc + (imm<<2));
						}else{
						EX_MEM.alures.write(ID_EX.npc);
						}									
					//ac_flush(IF);
					//ac_flush(ID);
                        break;
                case MEM:		
					npc = EX_MEM.alures.read(), 1;
                        break;
                case WB:
                        break;
                default:
       }  
/*		
  dbg_printf("blez r%d, %d\n", rs, imm & 0xFFFF);
  if( (RB[rs] == 0 ) || (RB[rs]&0x80000000 ) ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2), 1;
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }	*/
};

//!Instruction bgtz behavior method.
void ac_behavior( bgtz )
{
  dbg_printf("bgtz r%d, %d\n", rs, imm & 0xFFFF);
  if( !(RB[rs] & 0x80000000) && (RB[rs]!=0) ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }	
};

//!Instruction bltz behavior method.
void ac_behavior( bltz )
{
  dbg_printf("bltz r%d, %d\n", rs, imm & 0xFFFF);
  if( RB[rs] & 0x80000000 ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }	
};

//!Instruction bgez behavior method.
void ac_behavior( bgez )
{
  dbg_printf("bgez r%d, %d\n", rs, imm & 0xFFFF);
  if( !(RB[rs] & 0x80000000) ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }	
};

//!Instruction bltzal behavior method.
void ac_behavior( bltzal )
{
  dbg_printf("bltzal r%d, %d\n", rs, imm & 0xFFFF);
  RB[Ra] = ac_pc+4; //ac_pc is pc+4, we need pc+8
  if( RB[rs] & 0x80000000 ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }	
  dbg_printf("Return = %#x\n", ac_pc+4);
};

//!Instruction bgezal behavior method.
void ac_behavior( bgezal )
{
  dbg_printf("bgezal r%d, %d\n", rs, imm & 0xFFFF);
  RB[Ra] = ac_pc+4; //ac_pc is pc+4, we need pc+8
  if( !(RB[rs] & 0x80000000) ){
#ifndef NO_NEED_PC_UPDATE
    npc = ac_pc + (imm<<2);
#endif 
    dbg_printf("Taken to %#x\n", ac_pc + (imm<<2));
  }	
  dbg_printf("Return = %#x\n", ac_pc+4);
};

//!Instruction sys_call behavior method.
void ac_behavior( sys_call )
{
  dbg_printf("syscall\n");
  stop();
}

//!Instruction instr_break behavior method.
void ac_behavior( instr_break )
{
  fprintf(stderr, "instr_break behavior not implemented.\n"); 
  exit(EXIT_FAILURE);
}
