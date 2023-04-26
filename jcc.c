#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

enum tokenType
{
	identifier=1,
	integer,
	colon,
	semicolon,
	equal,
	oParenthesis,
	cParenthesis,
	oBrace,
	cBrace,
	comma,
	doubleQuote,
	singleQuote
};

char isSpecialChar(char ch)
{
	char specialChars[27] = {0, 0x20, 0x27, 0xa, 0x9, '!', '&', 0x27, '(', ')', '*', '+', ',', '-', '.', '/', ':', ';', '<', '=', '>', '[', ']', '{', '|', '}', '"'};
	for (int i = 0; i < sizeof(specialChars); i++)
	{
		if (ch == specialChars[i])
		{
			return 1;
		}
	}
	return 0;
}

char isOperand(char ch)
{
	char operandChars[10] = {'!', '*', '+', '-', '/', '<', '=', '>', '&', '|'};
	for (int i = 0; i < sizeof(operandChars); i++)
	{
		if (ch == operandChars[i])
		{
			return 1;
		}
	}
	return 0;
}

void tokenize(FILE* file, char*** tokens, int* nTokens, int** lineNums)
{
	char ch = 0;
	*nTokens = 0;
	int tokensize = 0;
	char prevCh = 0;
	while (ch != EOF)
	{
		ch = fgetc(file);
		if (!isSpecialChar(ch)) {prevCh = ch; continue;}
		if (!isSpecialChar(prevCh)) (*nTokens)++;
		if (ch != 0x20 && ch != 0x9 && ch != 0xa) (*nTokens)++;
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
		if (!isSpecialChar(ch))
		{
			tempStrSize++;
			prevCh = ch;
			continue;
		}
		if (!isSpecialChar(prevCh))
		{
			(*lineNums)[tokenPtr] = lineCounter;
			(*tokens)[tokenPtr] = malloc(tempStrSize + 1);
			if (ch == 0xa) {fseek(file, -(tempStrSize + 2), SEEK_CUR);}
			else {fseek(file, - (tempStrSize + 1), SEEK_CUR);}
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

char isInteger(char* token)
{
	char integerChars[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
	char* modifiedToken = token;
	char isValid;
	if (strlen(token) > 2)
	{
		if (token[0] == '0' && (token[1] == 'x' || token[1] == 'b'))
		{
			modifiedToken += 2;
		}
	}
	for (int i = 0; i < strlen(modifiedToken); i++)
	{
		isValid = 0;
		for (int j = 0; j < sizeof(integerChars); j++)
		{
			if (modifiedToken[i] == integerChars[j])
			{
				isValid = 1;
			}
		}
		if (!isValid) return 0;
	}
	return 1;
}

char isIdentifier(char* token)
{
	for (int i = 0; i < strlen(token); i++)
	{
		if (isSpecialChar(token[i])) return 0;
	}
	if (isInteger(token))
	{
		return 0;
	}
	if (!strcmp(token, "return"))
	{
		return 0;
	}
	if (!strcmp(token, "if"))
	{
		return 0;
	}
	return 1;
}

char getTokenType(char* token)
{
	if (strlen(token) == 1)
	{
		if (*token == ':') return colon;        // :
		if (*token == ';') return semicolon;    // ;
		if (*token == '=') return equal;        // =
		if (*token == '(') return oParenthesis; // (
		if (*token == ')') return cParenthesis; // )
		if (*token == '{') return oBrace;       // {
		if (*token == '}') return cBrace;       // }
		if (*token == ',') return comma;        // ,
		if (*token == '"') return doubleQuote;  // "
		if (*token == 0x22) return singleQuote; // '
	}
	if (isInteger(token)) return integer;
	if (isIdentifier(token)) return identifier;
	return 0;
}

// <funcDec>:    <identifier> ( <params> ) { <statements> }
// <params>:     <param> | <param> , <params>
// <param>:      <identifier> : <integer>
//
// <statements>: <statement> ; | <statement> ; <statements>
// <statement>:  <varDec> | <varAssign> | <if> | <funcCall> | <jump> | <return>
// <memAssign>:  <identifier> : <integer> | [ <identifier> ] : <integer>
// <memSet>:     <identifier> = <expr> | <varDec> = <expr>
// <if>:         if ( <expr> ) { <statements> }
// <expr>:       integer | identifier | <expr> + <expr> | <expr> - <expr> | <expr> * <expr> | <expr> / <expr> | <expr> && <expr> | !<expr> | <expr> || <expr> | <expr> == <expr> | <expr> != <expr> | <expr> < <expr> | <expr> > <expr> |
// 				 <expr> <= <expr> | <expr> >= <expr>
// <funcCall>:   <identifier> ( <args> )
// <jump>:       <identifier>
// <string>:     " ? "
// <return>:     return <expr>

int raiseError(const char* expectedToken, char* token, int lineNum)
{
	printf("Syntax error on line %d: unexpected token '%s'. Expected %s token.", lineNum, token, expectedToken);
	exit(0);
}

int parseVarDec(int* tokenPos, char** tokens, int* lineNums)
{
	int tokenOffset = 0;
	char* token = tokens[*tokenPos];

	if (getTokenType(token) != identifier)
	{
		return 0;
	}
	printf("%s\n", token);
	tokenOffset++;
	
	token = tokens[*tokenPos + tokenOffset];
	if (getTokenType(token) != colon)
	{
		return 0;
	}
	printf("%s\n", token);
	tokenOffset++;
	
	token = tokens[*tokenPos + tokenOffset];
	if (getTokenType(token) != integer)
	{
		return 0;
	}
	printf("%s\n", token);
	tokenOffset++;

	*tokenPos += tokenOffset;
	return 1;
}

int parseStatements(int tokenPos, char** tokens, int* lineNums)
{
	int tokenOffset = 0;
	char* token = tokens[tokenPos];

	if (parseVarDec(&tokenPos, tokens, lineNums))
	{
		return tokenPos;
	}
	printf("Syntax error on line %d: unexpected token '%s'. Expected statement.", lineNums[tokenPos], tokens[tokenPos]);
	exit(0);
}

int parseParams(int tokenPos, char** tokens, int* lineNums)
{
	int tokenOffset = 0;
	char* token = tokens[tokenPos];

	if (getTokenType(token) != identifier)
	{
		raiseError("identifier", token, lineNums[tokenPos + tokenOffset]);
	}
	printf("%s\n", token);
	tokenOffset++;
	
	token = tokens[tokenPos + tokenOffset];
	if (getTokenType(token) != colon)
	{
		raiseError(":", token, lineNums[tokenPos + tokenOffset]);
	}
	printf("%s\n", token);
	tokenOffset++;

	token = tokens[tokenPos + tokenOffset];
	if (getTokenType(token) != integer)
	{
		raiseError("integer", token, lineNums[tokenPos + tokenOffset]);
	}
	printf("%s\n", token);
	tokenOffset++;

	token = tokens[tokenPos + tokenOffset];
	if (getTokenType(token) != comma)
	{
		return tokenPos + tokenOffset;
	}
	printf("%s\n", token);
	tokenOffset++;

	return parseParams(tokenPos + tokenOffset, tokens, lineNums);
}

int parseFunc(int tokenPos, char** tokens, int* lineNums)
{
	int tokenOffset = 0;
	char* token = tokens[tokenPos];
	
	if (getTokenType(token) != identifier)
	{
		raiseError("identifier", token, lineNums[tokenPos + tokenOffset]);
	}
	printf("%s\n", token);
	tokenOffset++;

	token = tokens[tokenPos + tokenOffset];
	if (getTokenType(token) != oParenthesis)
	{
		raiseError("'('", token, lineNums[tokenPos + tokenOffset]);
	}
	printf("%s\n", token);
	tokenOffset++;

	token = tokens[tokenPos + tokenOffset];
	if (getTokenType(token) != cParenthesis)
	{
		tokenOffset = parseParams(tokenPos + tokenOffset, tokens, lineNums);
		token = tokens[tokenPos + tokenOffset];
		if (getTokenType(token) != cParenthesis)
		{
			raiseError("')'", token, lineNums[tokenPos + tokenOffset]);
		}
	}
	printf("%s\n", token);
	tokenOffset++;

	token = tokens[tokenPos + tokenOffset];
	if (getTokenType(token) != oBrace)
	{
		raiseError("'{'", token, lineNums[tokenPos + tokenOffset]);
	}
	printf("%s\n", token);
	tokenOffset++;

	token = tokens[tokenPos + tokenOffset];
	if (getTokenType(token) != cBrace)
	{
		tokenOffset = parseStatements(tokenPos + tokenOffset, tokens, lineNums);
		token = tokens[tokenPos + tokenOffset];
		if (getTokenType(token) != cBrace)
		{
			raiseError("'}'", token, lineNums[tokenPos + tokenOffset]);
		}
	}
	printf("%s\n", token);
	tokenOffset++;

	return tokenPos + tokenOffset;
}

int parse(char** tokens, int nTokens, int* lineNums)
{
	for (int i = 0; i < nTokens; i++)
	{
		i = parseFunc(i, tokens, lineNums);
	}
	return 0;
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
	printf("Hello, World!");
	return 0;
}
