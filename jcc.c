#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char cmpStr(const char* str1, const char* str2, int* index);
char isSpecialChar(char ch);
char inQuotesToggle(char inQuotes, char prevCh);
void tokenize(FILE* file, char*** tokens, int* nTokens, int** lineNums);
void raiseError(const char* expectedToken, char** tokens, int index, int* lineNums);
char parseIdentifier(char** tokens, int* index);
char parseLetter(char ch);
char parseDigit(char ch);
char parseBit(char ch);
char parseNybl(char ch);
char parseInteger(char** tokens, int* index);
char parsePType(char** tokens, int* index);
char parseChar(char** tokens, int* index);
char parseString(char** tokens, int* index);
char parseOp(char** tokens, int* index);
char parseAsgnOp(char** tokens, int* index);
char parseIncDec(char** tokens, int* index, int* lineNums);
char parseValues(char** tokens, int* index, int* lineNums);
char parseFunc(char** tokens, int* index, int* lineNums);
char parseNegate(char** tokens, int* index, int* lineNums);
char parseAddress(char** tokens, int* index, int* lineNums);
char parseDeref(char** tokens, int* index, int* lineNums);
char parseParen(char** tokens, int* index, int* lineNums);
char parseMem(char** tokens, int* index, int* lineNums);
char parseOper(char** tokens, int* index, int* lineNums);
char parseExpr(char** tokens, int* index, int* lineNums);
char parseScope(char** tokens, int* index, int* lineNums);
char parseElse(char** tokens, int* index, int* lineNums);
char parseIf(char** tokens, int* index, int* lineNums);
char parseAsgn(char** tokens, int* index, int* lineNums);
char parseTypeDec(char** tokens, int* index, int* lineNums);
char parseMemAsgn(char** tokens, int* index, int* lineNums);
char parseFuncStmt(char** tokens, int* index, int* lineNums);
char parseLabel(char** tokens, int* index, int* lineNums);
char parseJmp(char** tokens, int* index, int* lineNums);
char parseWhile(char** tokens, int* index, int* lineNums);
char parseStmt(char** tokens, int* index, int* lineNums);
char parseParams(char** tokens, int* index, int* lineNums);
char parseFuncDec(char** tokens, int* index, int* lineNums);
char parse(char** tokens, int nTokens, int* lineNums);

char isSpecialChar(char ch)
{
	char specialChars[] = {'\0', ' ', '\n', '\t', '!', '"', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', ':', ';', '<', '=', '>', '[', ']', '{', '|', '}', '@'};
	for (int i = 0; i < sizeof(specialChars); i++)
	{
		if (ch == specialChars[i])
		{
			return 1;
		}
	}
	return 0;
}

char inQuotesToggle(char inQuotes, char prevCh)
{
    if (inQuotes && prevCh != '\\') return 0;
    else if (!inQuotes) return 1;
}

void tokenize(FILE* file, char*** tokens, int* nTokens, int** lineNums)
{
	char ch = 0;
	*nTokens = 0;
	char prevCh = 0;
    char inSQuotes = 0;
    char inDQuotes = 0;

	while (ch != EOF)
	{
		ch = fgetc(file);

        if (ch == '\'') inSQuotes = inQuotesToggle(inSQuotes, prevCh);
        if (ch == '"') inDQuotes = inQuotesToggle(inDQuotes, prevCh);

		if (!isSpecialChar(ch) || inSQuotes || inDQuotes)
        {
            prevCh = ch;
            continue;
        }
        if (!isSpecialChar(prevCh)) (*nTokens)++;
		if (ch != ' ' && ch != '\t' && ch != '\n' && isSpecialChar(ch)) (*nTokens)++;
		prevCh = ch;
	}

	prevCh = 0;
	ch = 0;

	fseek(file, 0, SEEK_SET);

	*tokens = malloc((*nTokens) * sizeof(char*));
	*lineNums = malloc((*nTokens) * sizeof(int));

	int tokenPtr = 0;
	int tempStrSize = 0;
	int lineCounter = 1;
    inSQuotes = 0;
    inDQuotes = 0;
    
	while (ch != EOF)
	{
		ch = fgetc(file);
		if (ch == EOF)
		{
			if (!isSpecialChar(prevCh))
			{
				(*lineNums)[tokenPtr] = lineCounter;
				(*tokens)[tokenPtr] = malloc(tempStrSize + 1);
				fseek(file, ftell(file) - tempStrSize, SEEK_SET);
				for (int i = 0; i < tempStrSize; i++)
				{
					(*tokens)[tokenPtr][i] = (char)fgetc(file);
				}
				(*tokens)[tokenPtr][tempStrSize] = 0;
			}
			continue;
		}
        
        if (ch == '\'') inSQuotes = inQuotesToggle(inSQuotes, prevCh);
        if (ch == '"') inDQuotes = inQuotesToggle(inDQuotes, prevCh);

		if (!isSpecialChar(ch) || inSQuotes || inDQuotes)
		{
			tempStrSize++;
			prevCh = ch;
			continue;
		} else if (ch == '\'' || ch == '"')
        {
            tempStrSize++;
			(*lineNums)[tokenPtr] = lineCounter;
			(*tokens)[tokenPtr] = malloc(tempStrSize + 1);
			fseek(file, ftell(file) - tempStrSize, SEEK_SET);
			for (int i = 0; i < tempStrSize; i++)
			{
				(*tokens)[tokenPtr][i] = (char)fgetc(file);
			}
			prevCh = ch;
			(*tokens)[tokenPtr][tempStrSize] = 0;
			tempStrSize = 0;
			tokenPtr++;
            continue;
        }
        if (!isSpecialChar(prevCh))
		{
			(*lineNums)[tokenPtr] = lineCounter;
			(*tokens)[tokenPtr] = malloc(tempStrSize + 1);
			if (ch == 0xa) {fseek(file, ftell(file) - (tempStrSize + 2), SEEK_SET);}
			else {fseek(file, ftell(file) - (tempStrSize + 1), SEEK_SET);}
			for (int i = 0; i < tempStrSize; i++)
			{
				(*tokens)[tokenPtr][i] = (char)fgetc(file);
			}
			prevCh = fgetc(file);
			(*tokens)[tokenPtr][tempStrSize] = 0;
			tempStrSize = 0;
			tokenPtr++;
		}
		if (ch != 0x20 && ch != 0x9 && ch != 0xa)
		{
			if (isSpecialChar(prevCh)) prevCh = ch;
			(*lineNums)[tokenPtr] = lineCounter;
			(*tokens)[tokenPtr] = malloc(2);
			(*tokens)[tokenPtr][0] = ch;
			(*tokens)[tokenPtr][1] = 0;
			tokenPtr++;
			continue;
		}
		if (ch == 0xa) lineCounter++;
		prevCh = ch;
	}
	for (int i = 0; i < *nTokens; i++)
	{
		printf("%d: %s\n", (*lineNums)[i], (*tokens)[i]);
	}
}

void raiseError(const char* expectedToken, char** tokens, int index, int* lineNums)
{
	printf("Syntax error on line %d: unexpected token '%s'. Expected %s.\n", lineNums[index], tokens[index], expectedToken);
	exit(0);
}

char cmpStr(const char* str1, const char* str2, int* index)
{
    if (!strcmp(str1, str2)) (*index)++;
    return !strcmp(str1, str2);
}

char parseIdentifier(char** tokens, int* index)
{
    char* types[16] = { "i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64", "i128", "u128", "f32", "f64", "f80", "f128", "usize", "&" };
    for (int i = 0; i < 16; i++)
    {
        if (!strcmp(tokens[*index], types[i])) return 0;
    }
	if (!strcmp(tokens[*index], "return") || !strcmp(tokens[*index], "if") || !strcmp(tokens[*index], "while")) return 0;
	if (!parseLetter(tokens[*index][0]))
        if (tokens[*index] != '_') return 0;
	for (int i = 1; i < strlen(tokens[*index]); i++)
	{
		if (!parseLetter(tokens[*index][i]) && !parseDigit(tokens[*index][i]) && tokens[*index][i] != '_') return 0;
	}
    (*index)++;
	return 1;
}

char parseLetter(char ch)
{
	char letters[52] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
	for (int i = 0; i < 52; i++)
	{
		if (ch == letters[i]) return 1;
	}
	return 0;
}

char parseDigit(char ch)
{
	char digits[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	for (int i = 0; i < 10; i++)
	{
		if (ch == digits[i]) return 1;
	}
	return 0;
}

char parseBit(char ch)
{
    if (ch == '0' || ch == '1') return 1;
    return 0;
}

char parseNybl(char ch)
{
    char nybls[22] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F'};
    for (int i = 0; i < 22; i++)
    {
        if (ch == nybls[i]) return 1;
    }
    return 0;
}

char parseInteger(char** tokens, int* index)
{
	int i = 0;
    int len = 2;
	if (tokens[*index][i] == '-') { i++; len++; }
    if (strlen(tokens[*index]) > len)
    {
        if (tokens[*index][i] == '0' && tokens[*index][i + 1] == 'x')
        {
            for (int j = i + 2; j < strlen(tokens[*index]); j++)
            {
                if (!parseNybl(tokens[*index][j])) return 0;
            }
            (*index)++;
            return 1;
        } else if (tokens[*index][i] == '0' && tokens[*index][i + 1] == 'b')
        {
            for (int j = i + 2; j < strlen(tokens[*index]); j++)
            {
                if (!parseBit(tokens[*index][j])) return 0;
            }
            (*index)++;
            return 1;
        }
    }
    for (int j = 0; j < strlen(tokens[*index]); j++)
    {
        if (!parseDigit(tokens[*index][i + j])) return 0;
    }
    (*index)++;
	return 1;
}

char parsePType(char** tokens, int* index)
{
    char* types[16] = { "i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64", "i128", "u128", "f32", "f64", "f80", "f128", "usize", "&" };
    for (int i = 0; i < 16; i++)
    {
        if (!strcmp(tokens[*index], types[i])) {(*index)++; return 1;}
    }
	return 0;
}

char parseChar(char** tokens, int* index)
{
	if (strlen(tokens[*index]) != 3) return 0;
	if (tokens[*index][0] != 0x27 && tokens[*index][2] != 0x27) return 0;
	if (tokens[*index][1] == 0x27) return 0;
    (*index)++;
	return 1;
}

char parseString(char** tokens, int* index)
{
	if (tokens[*index][0] != '"' && tokens[*index][strlen(tokens[*index]) - 1] != '"') return 0;
	for (int i = 1; i < strlen(tokens[*index]) - 1; i++)
	{
		if (tokens[*index][i] == '"') return 0;
	}
    (*index)++;
	return 1;
}

char parseOp(char** tokens, int* index)
{
	if (!strcmp(tokens[*index], "+")
		|| !strcmp(tokens[*index], "-")
		|| !strcmp(tokens[*index], "*")
		|| !strcmp(tokens[*index], "/")
		|| !strcmp(tokens[*index], "%")
		|| !strcmp(tokens[*index], "<")
		|| !strcmp(tokens[*index], ">")
		|| !strcmp(tokens[*index], "&")
		|| !strcmp(tokens[*index], "^")
		|| !strcmp(tokens[*index], "|")) {(*index)++; return 1;}
	if ((!strcmp(tokens[*index], "=") && !strcmp(tokens[*index + 1], "="))
		|| (!strcmp(tokens[*index], "!") && !strcmp(tokens[*index + 1], "="))
		|| (!strcmp(tokens[*index], "<") && !strcmp(tokens[*index + 1], "="))
		|| (!strcmp(tokens[*index], ">") && !strcmp(tokens[*index + 1], "="))
		|| (!strcmp(tokens[*index], "&") && !strcmp(tokens[*index + 1], "&"))
		|| (!strcmp(tokens[*index], "|") && !strcmp(tokens[*index + 1], "|"))
		|| (!strcmp(tokens[*index], "<") && !strcmp(tokens[*index + 1], "<"))
		|| (!strcmp(tokens[*index], ">") && !strcmp(tokens[*index + 1], ">"))) {(*index) += 2; return 1;}
	return 0;
}

char parseAsgnOp(char** tokens, int* index)
{
	if (!strcmp(tokens[*index], "=")) {(*index)++; return 1;}
	if (!strcmp(tokens[*index + 1], "=") && (
		!strcmp(tokens[*index], "+")
		|| !strcmp(tokens[*index], "-")
		|| !strcmp(tokens[*index], "*")
		|| !strcmp(tokens[*index], "/")
		|| !strcmp(tokens[*index], "%")
		|| !strcmp(tokens[*index], "&")
		|| !strcmp(tokens[*index], "|")
		|| !strcmp(tokens[*index], "^"))) {(*index) += 2; return 1;}
    if (!strcmp(tokens[*index + 2], "=") && (
		(!strcmp(tokens[*index], "<") && !strcmp(tokens[*index + 1], "<"))
		|| (!strcmp(tokens[*index], ">") && !strcmp(tokens[*index + 1], ">")))) {(*index) += 3; return 1;}
	return 0;
}

char parseIncDec(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!parseIdentifier(tokens, &i)) return 0;
	if ((strcmp(tokens[i], "+") || strcmp(tokens[i + 1], "+"))
		&& (strcmp(tokens[i], "-") || strcmp(tokens[i + 1], "-"))) return 0; i += 2;

	(*index) = i;
	return 1;
}

char parseValues(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!parseExpr(tokens, &i, lineNums)) return 0;

	while (cmpStr(tokens[i], ",", &i))
	{
		if (!parseExpr(tokens, &i, lineNums)) {raiseError("expression", tokens, i, lineNums);}
	}

	(*index) = i;
	return 1;
}

char parseFunc(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!parseIdentifier(tokens, &i)) return 0;
	if (!cmpStr(tokens[i], "(", &i)) return 0;
	parseValues(tokens, &i, lineNums);
	if (!cmpStr(tokens[i], ")", &i)) return 0;

	(*index) = i;
	return 1;
}

char parseNegate(char** tokens, int* index, int* lineNums)
{
	int i = *index;

	if (!cmpStr(tokens[i], "!", &i) && !cmpStr(tokens[i], "~", &i)) return 0;
	if (!parseOper(tokens, &i, lineNums)) return 0;

	*index = i;
	return 1;
}

char parseAddress(char** tokens, int* index, int* lineNums)
{
	int i = *index;

	if (!cmpStr(tokens[i], "&", &i)) return 0;
	if (parseIdentifier(tokens, &i)) goto parseAddressEnd;
    if (parseMem(tokens, &i, lineNums)) goto parseAddressEnd;
    if (parseString(tokens, &i)) goto parseAddressEnd;
    return 0;

    parseAddressEnd:
	*index = i;
	return 1;
}

char parseDeref(char** tokens, int* index, int* lineNums)
{
	int i = *index;

	if (!cmpStr(tokens[i], "[", &i)) return 0;
	if (!parseExpr(tokens, &i, lineNums)) return 0;
	if (!cmpStr(tokens[i], "]", &i)) return 0;

	*index = i;
	return 1;
}

char parseParen(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	
	if (!cmpStr(tokens[i], "(", &i)) return 0;
	if (!parseExpr(tokens, &i, lineNums)) return 0;
	if (!cmpStr(tokens[i], ")", &i)) return 0;

	*index = i;
	return 1;
}

char parseMem(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	
	if (!cmpStr(tokens[i], "{", &i)) return 0;
	parseValues(tokens, &i, lineNums);
	if (!cmpStr(tokens[i], "}", &i)) return 0;

	*index = i;
	return 1;
}

char parseOper(char** tokens, int* index, int* lineNums)
{
	if (parseNegate(tokens, index, lineNums)) return 1;
	if (parseFunc(tokens, index, lineNums)) return 1;
	if (parseDeref(tokens, index, lineNums)) return 1;
	if (parseParen(tokens, index, lineNums)) return 1;
	if (parseMem(tokens, index, lineNums)) return 1;
	if (parseIncDec(tokens, index, lineNums)) return 1;
	if (parseAddress(tokens, index, lineNums)) return 1;
	if (parseIdentifier(tokens, index)) return 1;
	if (parseInteger(tokens, index)) return 1;
	if (parseChar(tokens, index)) return 1;
	if (parseString(tokens, index)) return 1;
	if (parsePType(tokens, index)) return 1;
    if (cmpStr(tokens[*index], "_", index)) return 1;

	return 0;
}

char parseExpr(char** tokens, int* index, int* lineNums)
{
	int i = *index;

	if (!parseOper(tokens, &i, lineNums)) return 0;
    parseTypeDec(tokens, &i, lineNums);

	while (parseOp(tokens, &i))
	{
		if (!parseOper(tokens, &i, lineNums)) raiseError("operand", tokens, i, lineNums);
        parseTypeDec(tokens, &i, lineNums);
	}

	(*index) = i;
	return 1;
}

char parseScope(char** tokens, int* index, int* lineNums)
{
	int i = *index;

	if (!cmpStr(tokens[i], "{", &i)) return 0;
	while (!cmpStr(tokens[i], "}", &i))
	{
		if (!parseStmt(tokens, &i, lineNums)) raiseError("statement", tokens, i, lineNums);
	}
    
	(*index) = i;
	return 1;
}

char parseElse(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!cmpStr(tokens[i], "else", &i)) return 0;
	if (!parseStmt(tokens, &i, lineNums)) return 0;;

	(*index) = i;
	return 1;
}

char parseIf(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!cmpStr(tokens[i], "if", &i)) return 0;
	if (!cmpStr(tokens[i], "(", &i)) raiseError("( token", tokens, i, lineNums);
	if (!parseExpr(tokens, &i, lineNums)) raiseError("expression", tokens, i, lineNums);
	if (!cmpStr(tokens[i], ")", &i)) raiseError(") token", tokens, i, lineNums);
	if (!parseStmt(tokens, &i, lineNums)) raiseError("statement", tokens, i, lineNums);
	parseElse(tokens, &i, lineNums);

	(*index) = i;
	return 1;
}

char parseAsgn(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!cmpStr(tokens[i], "=", &i)) return 0;
	if (!parseExpr(tokens, &i, lineNums)) return 0;

	(*index) = i;
	return 1;
}

char parseReAsgn(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!parseAsgnOp(tokens, &i)) return 0;
	if (!parseExpr(tokens, &i, lineNums)) return 0;

	(*index) = i;
	return 1;
}

char parseTypeDec(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!cmpStr(tokens[i], ":", &i)) return 0;
	if (!parseExpr(tokens, &i, lineNums)) return 0;

	(*index) = i;
	return 1;
}

char parseVarAsgn(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!parseIdentifier(tokens, &i)) return 0;
	if (!cmpStr(tokens[i], ":", &i)) return 0;
	parseExpr(tokens, &i, lineNums);
	parseAsgn(tokens, &i, lineNums);
	if (!cmpStr(tokens[i], ";", &i)) return 0;

	(*index) = i;
	return 1;
}

char parseVReAsgn(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!parseIdentifier(tokens, &i)) return 0;
	parseReAsgn(tokens, &i, lineNums);
	if (!cmpStr(tokens[i], ";", &i)) return 0;

	(*index) = i;
	return 1;
}

char parseMemAsgn(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!parseDeref(tokens, &i, lineNums)) return 0;
	if (!parseTypeDec(tokens, &i, lineNums)) return 0;
	if (!parseAsgn(tokens, &i, lineNums)) return 0;
	if (!cmpStr(tokens[i], ";", &i)) return 0;

	(*index) = i;
	return 1;
}

char parseFuncStmt(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!parseFunc(tokens, &i, lineNums)) return 0;
	if (!cmpStr(tokens[i], ";", &i)) raiseError("; token", tokens, i, lineNums);

	(*index) = i;
	return 1;
}

char parseLabel(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!parseIdentifier(tokens, &i)) return 0;
	if (!cmpStr(tokens[i], ":", &i)) return 0;

	(*index) = i;
	return 1;
}

char parseJmp(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!parseIdentifier(tokens, &i)) return 0;
	if (!cmpStr(tokens[i], ";", &i)) return 0;

	(*index) = i;
	return 1;
}

char parseWhile(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!cmpStr(tokens[i], "while", &i)) return 0;
	if (!cmpStr(tokens[i], "(", &i)) raiseError("( token", tokens, i, lineNums);
	if (!parseExpr(tokens, &i, lineNums)) raiseError("expression", tokens, i, lineNums);
	if (!cmpStr(tokens[i], ")", &i)) raiseError(") token", tokens, i, lineNums);
	if (!parseStmt(tokens, &i, lineNums)) raiseError("statement", tokens, i, lineNums);

	(*index) = i;
	return 1;
}

char parseStmt(char** tokens, int* index, int* lineNums)
{
	if (parseIf(tokens, index, lineNums)) return 1;
	if (parseScope(tokens, index, lineNums)) return 1;
	if (parseVarAsgn(tokens, index, lineNums)) return 1;
	if (parseVReAsgn(tokens, index, lineNums)) return 1;
	if (parseMemAsgn(tokens, index, lineNums)) return 1;
	if (parseFuncStmt(tokens, index, lineNums)) return 1;
	if (parseLabel(tokens, index, lineNums)) return 1;
	if (parseJmp(tokens, index, lineNums)) return 1;
	if (parseWhile(tokens, index, lineNums)) return 1;
    if (parseExpr(tokens, index, lineNums)) return 1;
	if (cmpStr(tokens[*index], ";", index)) return 1;

	return 0;
}

char parseParam(char** tokens, int* index, int* lineNums)
{
	int i = *index;

    cmpStr(tokens[i], "&", &i);
	if (!parseIdentifier(tokens, &i)) return 0;
    if (!parseTypeDec(tokens, &i, lineNums)) return 0;

	(*index) = i;
	return 1;
}

char parseParams(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!parseParam(tokens, &i, lineNums)) return 0;

	while (cmpStr(tokens[i], ",", &i))
	{
		i++;
		if (!parseParam(tokens, &i, lineNums)) return 0;
	}

	(*index) = i;
	return 1;
}

char parseFuncDec(char** tokens, int* index, int* lineNums)
{
	int i = *index;
	if (!parseIdentifier(tokens, &i)) return 0;
	if (!cmpStr(tokens[i], "(", &i)) raiseError("( token", tokens, i, lineNums);
	parseParams(tokens, &i, lineNums);
	if (!cmpStr(tokens[i], ")", &i)) raiseError(") token", tokens, i, lineNums);
	parseTypeDec(tokens, &i, lineNums);
	if (!parseStmt(tokens, &i, lineNums)) raiseError("statement", tokens, i, lineNums); return 0;

	(*index) = i;
	return 1;
}

char parse(char** tokens, int nTokens, int* lineNums)
{
	int i = 0;
	while (i < nTokens)
	{
		if (!parseFuncDec(tokens, &i, lineNums)) break;
	}
}

int main()
{
	char** tokens;
	int nTokens;
	int* lineNums;
	FILE* file;
	file = fopen("main.jc", "r");
	if (file == NULL)
	{
		printf("File can't be opened\n\n");
		return 1;
	}
	tokenize(file, &tokens, &nTokens, &lineNums);
	fclose(file);
	parse(tokens, nTokens, lineNums);
	free(tokens);
	return 0;
}
