#include "lexer.h"

#include "smierror.h"

#include <algorithm>
#include <sstream>

const std::vector<std::string> INSTRUCTIONS = { "MOV", "ADD", "CMP", "BEQ" };

namespace smi::lexer {

int tokenize(const std::string& code, Tokens& tokens) {
	int pos = 0;
	int line = 1;
	int lineStart = 0;

	while (pos < code.length()) {
		char ch = code[pos];
		std::string val;

		switch (ch) {
			case '\r': case '\t': case ' ':
				break;
			case '\n':
				line++;
				lineStart = pos + 1;
				break;
			case ',':
				tokens.push_back(TOKEN_NEW(TokenType::COMMA, ",", pos, line, pos - lineStart + 1));
				break;
			default:
				val = "";
				int valStartPos = pos;

				while (std::isalnum(code[pos])) {
					val += code[pos];
					pos++;
				}

				if (val.length() == 0) {
					std::ostringstream buffer;
					buffer << "Invalid token '" << ch << "' at position " << pos << " (line: " << line << ", column: " << (pos - lineStart + 1) << ")";
					
					int lineEnd = pos;					
					while (lineEnd < code.length() - 1 && code[lineEnd] != '\n')
						lineEnd++;
					
					buffer << std::endl << code.substr(lineStart, lineEnd - lineStart);
					buffer << std::endl << std::string(pos - lineStart, '-') << "^";

					setLastError(buffer.str());
					return LEXER_ERR_INVALID_TOKEN;
				}

				if (code[pos] == ':') {
					tokens.push_back(TOKEN_NEW(TokenType::LABEL, val, valStartPos, line, valStartPos - lineStart + 1));
				}
				else if (std::find(INSTRUCTIONS.begin(), INSTRUCTIONS.end(), val) != INSTRUCTIONS.end()) {
					tokens.push_back(TOKEN_NEW(TokenType::INSTR, val, valStartPos, line, valStartPos - lineStart + 1));
					pos--;
				}
				else {
					if (tokens.size() > 0 && tokens[tokens.size() - 1].type == TokenType::LABEL) {
						tokens.push_back(TOKEN_NEW(TokenType::LITERAL, val, valStartPos, line, valStartPos - lineStart + 1));
					}
					else {
						tokens.push_back(TOKEN_NEW(TokenType::OP, val, valStartPos, line, valStartPos - lineStart + 1));
					}
					
					pos--;
				}
		}

		pos++;
	}

	return LEXER_OK;
}

}
