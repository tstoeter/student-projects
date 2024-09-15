public enum TokenType
{
	INVALID,
	VALID,
	NUMBER,
	SYMBOL,
	FUNCTION,
	PARANTHESIS,
	VARIABLE;

	// gucken zu welchem tokentyp ein zeichen gehört
	public static TokenType charType(char ch)
	{
		// zahlen bestehen aus ziffern und komma bzw. punkt
		if (Character.getType(ch) == Character.DECIMAL_DIGIT_NUMBER || ch == '.')
		{
			return NUMBER;
		}
		// mathematische symbole
		else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^')
		{
			return SYMBOL;
		}
		// funktionen als strings, liegt nur in kleinbuchstaben vor
		else if (Character.getType(ch) == Character.LOWERCASE_LETTER)
		{
			return FUNCTION;
		}
		// klammern auf und zu
		else if (ch == '(' || ch == ')')
		{
			return PARANTHESIS;
		}
		// alle anderen zeichen sind undefiniert
		else
		{
			return INVALID;
		}
	}
}

