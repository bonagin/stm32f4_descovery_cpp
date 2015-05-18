#include <cyg/kernel/diag.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "TermCMD.h"
#include "utils.h"

void showThread(cTerm & t);
void showUsage(cTerm & t);

void System::handle(cTerm & t,int argc,char *argv[])
{
	if (!strcmp(argv[0],"dt"))
	{
		showThread(t);
		showUsage(t);
	}

	if(!strcmp(argv[0],"time"))
	{

		time_t now = time(NULL);
		t << "wallclock: " << asctime(localtime(&now));

		now = cyg_current_time() / 100;
		if(now < 60)
		{
			t << "Up time  : " << (int)now << "s";
		}
		else
		{
			int min = now / 60;
			now %= 60;
			if(min < 60)
			{
				t << "Up time  : " << min << "m" << (int)now << "s";
			}
			else
			{
				int hour = min / 60;
				min %= 60;
				if(hour < 24)
				{
					t << "Up time  : " << hour << "h" << min << "m" << (int)now << "s";
				}
				else
				{
					int day = hour / 24;
					hour %= 24;
					t << "Up time  :  "<< day << "d" << hour << "h" << min << "m" << (int)now << "s";
				}
			}
		}
	}
}


void showUsage(cTerm & t)
{
	extern cyg_uint32  __rom_data_start;	//diag_printf("ROMstart 0x%08X\n",(cyg_uint32)&__rom_data_start);

	extern cyg_uint32  __ram_data_start;	//diag_printf("RAMstart 0x%08X\n",(cyg_uint32)&__ram_data_start);
	extern cyg_uint32  __ram_data_end;		//diag_printf("RAMend   0x%08X\n",(cyg_uint32)&__ram_data_end);
	extern cyg_uint32  _end;				//diag_printf("__end    0x%08X\n",(cyg_uint32)&_end);
	struct mallinfo heap_info = mallinfo();

	cyg_uint32 text_size = (cyg_uint32)&__rom_data_start - 0x08000010; //this is the program address
	cyg_uint32 data_size =  (cyg_uint32)&__ram_data_end - (cyg_uint32)&__ram_data_start;
	cyg_uint32 bss_size  = (cyg_uint32)&_end - (cyg_uint32)&__ram_data_end + 8;
	cyg_uint32 total_rom = text_size + data_size + bss_size;
	cyg_uint32 usage = ((total_rom*100)/(0x100000-0x180));

	t << t.format(GREEN("\nTotal ROM: %d\n"), total_rom);
	t << t.format(" .text = %d\n", text_size);
	t << t.format(" .data = %d\n", data_size);
	t << t.format(" .bss  = %d\n", bss_size);
	t << t.format("        %d%%\n\n", usage);

	t << t.format(GREEN("Total RAM:\n"));
	t << t.format(" heap = %d\n", heap_info.arena);
	t << t.format(" Used = %d\n",heap_info.usmblks+heap_info.uordblks);
	t << t.format("        %d%%\n",(((heap_info.usmblks+heap_info.uordblks)*100)/heap_info.arena));

}

void showThread(cTerm & t)
{
	cyg_handle_t thread = 0;
	cyg_uint16 id;
	cyg_thread_info info;
	bool flag = 0;

	t << t.format(GREEN("ID %15s%6s%12s%12s%5s\n"), "Name", "Prior", "S_size", "Used", "Perc");

	while ( cyg_thread_get_next(&thread,&id) )
	{
		if ( !cyg_thread_get_info(thread,id,&info) )
		{
			break;
		}

		if(flag)
			t << t.format("%02d%16s%6d  0x%08X  0x%08X%5d\n", info.id, info.name, info.set_pri, info.stack_size, info.stack_used, (int)((info.stack_used*100)/info.stack_size));
		else
			t << t.format(CYAN("%02d%16s%6d  0x%08X  0x%08X%5d\n"), info.id, info.name, info.set_pri, info.stack_size, info.stack_used, (int)((info.stack_used*100)/info.stack_size));

		flag = !flag;
	}
}

void System::reset(cTerm & t, int argc, char *argv[])
{
	t << "Device will now RESET\n";

	cyg_thread_delay(100);
	cyg_scheduler_lock();

	HAL_WRITE_UINT32(0xE000ED00 + 0x0C, (0x5FA << 16) | (1 << 2));
}

void System::setTime(cTerm & term, int argc,char * argv[])
{
	cyg_uint16 yy,mm,dd,h,m,s;
	if(argc > 5)
	{
		yy = strtoul(argv[1],NULL,10);
		if(yy < 1900)
			return;
		term << term.format("yy : %d\n", yy);

		mm = strtoul(argv[2],NULL,10);
		if(mm > 12)
			return;
		term << term.format("mm : %d\n", mm);

		dd = strtoul(argv[3],NULL,10);
		if(dd > 31)
			return;
		term << term.format("dd : %d\n", dd);

		h = strtoul(argv[4],NULL,10);
		if(h > 24)
			return;
		term << term.format("h  : %d\n", h);

		m = strtoul(argv[5],NULL,10);
		if(m > 60)
			return;
		term << term.format("m  : %d\n", m);

		s = strtoul(argv[6],NULL,10);
		if(s > 60)
			return;
		term << term.format("s  : %d\n", s);

		time_t timeVal;
		struct tm info;
		info.tm_year = yy - 1900;
		info.tm_mon =  mm - 1;
		info.tm_mday = dd;
		info.tm_hour = h;
		info.tm_min = m;
		info.tm_sec = s;
		timeVal = mktime(&info);

		if(!cyg_libc_time_settime(timeVal))
			term << GREEN("Updated time\n");
		else
			term << RED("Could not update time\n");
	}
	else
	{
		term<<"Not enough input values. See help\n";
	}
}

TermCMD::TermCMD()
{
}

void TermCMD::process(cTerm & term, int argc,char * argv[])
{
	cmd_table_t* t_ptr = NULL;

	int k = 0;
	do
	{
		t_ptr = &mCmdTable[k++];
		if(!t_ptr->cmd)
			break;

		if(t_ptr->f && !strcmp(argv[0],t_ptr->cmd))
		{
			t_ptr->f(term, argc, argv);
			return;
		}
	}while(t_ptr->cmd);

	term << RED("Unknown Command. Type help for a list of commands\n");
}

void TermCMD::help(cTerm & t,int argc,char *argv[])
{

	t << YELLOW("TermCMD commands:\n");

	cmd_table_t* t_ptr = NULL;

	int k = 0;
	do
	{
		t_ptr = &mCmdTable[k++];
		if(!t_ptr->cmd)
			break;

		if(t_ptr->f)
		{
			t <<"  "<< t.format(CYAN("%-10s "),t_ptr->cmd) << t.format("%-5s ", t_ptr->argDesc) << t.format("- %s\n",t_ptr->desc);
		}
		else
		{
			//this is a caption
			t << t.format("%s\n", t_ptr->cmd);
		}

	}while(t_ptr->cmd);

}


