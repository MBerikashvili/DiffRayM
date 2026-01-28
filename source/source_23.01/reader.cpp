#include "reader.h"

int CReader::readLine(FILE *MF, char *string, int(*fn)(int))
{
	if(!feof(MF))
	{
		string = fgets(string,500,MF);
		fn(121);
		return 1;

	}
	else
	{
		return 0;
	}
};

char** CReader::read_commands(FILE *MF)
{

	char string[500];
	
	char **result;
	
	char *command_part;
	if(!feof(MF))
	{
		fgets(string,500,MF);
		while(string[0] == '\n')
		{
			fgets(string,500,MF);
			if(feof(MF))
			{
				return NULL;
			}
		}
		command_part = strtok(string," ,;-\t\r\n");
		int i=0;
		result = new char*[10];
		bool isQuoted = false;
		
		while(command_part != NULL)
		{
			char *remaining;
			remaining = strtok(NULL, "");
			result[i] = new char[255];
			strncpy(result[i],command_part,255);
			i++;
			if(remaining == NULL)
				break;

			if(remaining[0] == '\'')
			{
				command_part = strtok(remaining, "'");
			}
			else
			    command_part = strtok(remaining," ,;-\t\r\n");
		}
		if(i<10)
		{
			while(i < 10)
			{
				result[i] = new char[255];
				strncpy(result[i],"",255);
				i++;
			}
		}
		return result;

	}
	else
	{
		return NULL;
	}
}

int CReader::calc_lines_number(FILE *MF, int maxsize)
{
	char *str;
	str = new char[maxsize];
	int num = 0;
	
	while(fgets(str,maxsize,MF) != NULL)
	    ++num;

	printf("N: %d\n", num);
	delete str;
	return num;
}

int CReader::calc_cols_number(char *line, const char *pattern)
{
	char *ptr = line;
	char strdata[255];
	int icols = 0;
	int n;
	//	printf("pattern: %s\n", pattern);
	while ( sscanf(ptr, pattern, strdata, &n) == 1 )
    {
    	double val;
		//	printf("%s\n", strdata);
		sscanf(strdata,"%lf",&val);
		ptr += strlen(strdata)+1;
		icols++;
	}

	return icols;
}

int CReader::read_file_array(FILE *MF, int maxsize, int nskiprows, int nskipcols, int (*oncalcrows)(int), int (*oncalccols)(int), int (*onelementset)(int,int,double), int (*onskiprow)(int,char[1000]))
{
	if(!MF || MF == NULL)
	{
		CBasics::throwError("ERROR OPENING FILE WITH NULL POINTER!");
	}
	//first calculate number of lines
printf("NeOK-1\n");
	int num = CReader::calc_lines_number(MF, maxsize);
printf("NeOK-2\n");
	rewind(MF);
printf("NeOK-3\n");
	if(oncalcrows)
	{
		oncalcrows(num);
	}
printf("OK1!!!!\n");
	char *line;
	line = new char[maxsize];
printf("OK2!!!!\n");
	//result = new double*[num];
	double val;
	
	int irows = 0;
	while(irows<nskiprows && !feof(MF))
	{

		fgets(line,maxsize,MF);
		if(onskiprow)
		{
			onskiprow(irows,line);
		}
		
		irows++;
	}
	int numcols = -1;
	int irow = 0;
	int icol = 0;
	while(!feof(MF))
	{
		fgets(line,maxsize,MF);
		//get numb of colls
		if(numcols<0)
		{    
		    numcols = CReader::calc_cols_number(line,"%s[\t\n]");

			if(oncalccols)
			{
				oncalccols(numcols);
			}
		}
		//result[irow] = new double[numcols];

		//printf("MLL: %s\n",line);
		int n = 0;
		char *ptr = line;
		char strdata[255];
		int icols = 0;
		icol = 0;
		bool iSkip = false;

		while ( sscanf(ptr, "%s[\t\n]", strdata, &n) == 1 && !iSkip)
    	{
    	    //printf("offset: %d\n",strlen(strdata));
		    double val;
		    sscanf(strdata,"%lf",&val);
		    ptr += strlen(strdata)+1;
		    if(icols>=nskipcols)
		    {
				//result[irow][icol] = val;
				if(onelementset)
				{
					int code = onelementset(irow,icol,val);
					if(code == -1)
					{
						//printf("(%d) - %le\n", code, val);
						iSkip = true;
					}
				}
				icol++;
		    }
		    icols++;
		}
		irow++;
	}
	return 1;
}

int CReader::callable(int numb)
{
	printf("NUMBER: %d\n",numb);
	return 1;
}
