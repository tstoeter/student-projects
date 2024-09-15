import java.util.*;
import java.util.regex.*;

public class Tokenizer
{
	String str;
	LinkedList<Token> tokens;

	LinkedList<String> symtab;

	// konstruktor setzt eingabestring
	public Tokenizer(String in, LinkedList<String> st)
	{
		this.setInputString(in);
		symtab = st;
	}

	// eingabe string setzen und tokenliste initialisieren
	public void setInputString(String in)
	{
		// eingabestring umklammern, um z.B. -2+2 zu parsen, (- wird später zu (0- substituiert
		str = "(" + in.toLowerCase() + ")";
		tokens = new LinkedList<Token>();
	}

	public LinkedList<Token> getTokens()
	{
		return tokens;
	}

	// eingabesetring zeichenweise scannen und erkannte tokens in liste einfügen
	public void tokenize()
	{
		// substituiere (- mit (0-, vorzeichen wird zur binären operation
		Pattern pattern;
		Matcher matcher;

		pattern = Pattern.compile("\\(-");
		matcher = pattern.matcher(str);
		str = matcher.replaceAll("(0-");

		// auch mit + als vorzeichen
		pattern = Pattern.compile("\\(\\+");
		matcher = pattern.matcher(str);
		str = matcher.replaceAll("(0+");

		// string initialisieren
		String tokenStr = new String();

		// eingabe string in kleinbuchstaben umwandeln, da wir nur nach kleinbuchstaben vergleichen
		str.toLowerCase();

		// und erstes zeichen verarbeiten, damit erstes token nicht leer wird
		char ch = str.charAt(0);
		tokenStr += Character.toString(ch);

		TokenType curr_type = TokenType.charType(ch);
		TokenType last_type = TokenType.charType(ch);

		// über alle zeichen iterieren, beginnend mit dem zweiten
		for (int i=1; i<str.length(); i++)
		{
			// aktuelles zeichen lesen und klassifizieren
			ch = str.charAt(i);
			curr_type = TokenType.charType(ch);

			// undefinierte zeichen überspringen, somit auch whitespaces, aber auch unbekannte symbole werden gefiltert
			if (curr_type == TokenType.INVALID)
			{
				last_type = TokenType.INVALID;
				continue;
			}

			// funktionen und zahlen als zeicheketten einlesen
			if ((curr_type == TokenType.FUNCTION || curr_type == TokenType.NUMBER) && (curr_type == last_type))
			{
				tokenStr += Character.toString(ch);
			}
			// wenn neuer token typ anfängt oder einstelliges token
			else
			{
				// altes token in liste einfügen
				tokens.add(new Token(tokenStr, TokenType.charType(tokenStr.charAt(0))));

				// und neues beginnen
				tokenStr = new String();
				tokenStr += Character.toString(ch);
			}

			last_type = curr_type;
		}

		// letzten token auch hinzufügen
		tokens.add(new Token(tokenStr, last_type));

		// jetzt gefundene tokens überprüfen
		checkTokens();
	}

	public static int findInSymTab(LinkedList<String> st, String vs)
	{
		int index = -1;
		ListIterator<String> variter = st.listIterator(0);

		while (variter.hasNext())
		{
			String varstr = variter.next();

			if (vs.equals(varstr))
			{
				index = variter.nextIndex()-1;
				break;
			}
		}

		return index;
	}

	// tokens auf korrektheit prüfen
	public void checkTokens()
	{
		// iterator über tokenliste
		ListIterator<Token> iter = tokens.listIterator(0);

		// liste durchgehen
		while (iter.hasNext())
		{
			// und jeden einzelnen token betrachten
			Token token = iter.next();

			// token könnte variablen bezeichnung sein, suche in symtab
			Integer index = findInSymTab(symtab, token.str);

			if (index >= 0)
			{
				token = new Token(index.toString(), TokenType.VARIABLE);
				iter.set(token);
				continue;
			}
			
			char ch = token.str.charAt(0);

			// zahlen prüfen
			if (TokenType.charType(ch) == TokenType.NUMBER)
			{
				// versuche double zu parsen, numberformatexception bei fehler
				Double.parseDouble(token.str);
				token.type = TokenType.NUMBER;
			}
			// funktionsnamen prüfen
			else if (TokenType.charType(ch) == TokenType.FUNCTION)
			{
				if (FunctionStrings.isFunctionString(token.str))
					token.type = TokenType.FUNCTION;
				else
					throw new IllegalArgumentException("unrecognized token: " + token.str);
			}
			// bei unbekanntem token ebenfalls exception werfen
			else if (TokenType.charType(ch) == TokenType.INVALID)
			{
				throw new IllegalArgumentException("unrecognized token: " + token.str);
			}				
		}
	}

	public String getSyntaxString()
	{
		// iterator über tokenliste
		ListIterator<Token> iter = tokens.listIterator(0);
		String syntaxstr = new String();

		// liste durchgehen
		while (iter.hasNext())
		{
			// und jeden einzelnen token betrachten
			Token token = iter.next();

			if (token.type == TokenType.NUMBER || token.type == TokenType.VARIABLE)
				syntaxstr += "expr";
			else if (token.type == TokenType.PARANTHESIS)
				syntaxstr += token.str;
			else if (token.type == TokenType.FUNCTION)
				syntaxstr += "fn";
			else if (token.type == TokenType.SYMBOL)
				syntaxstr += "&";
		}

		return syntaxstr;
	}

	// ausgabe von eingabestring und tokenliste zu debug-zwecken
	public String toString()
	{
		return tokens.toString();
	}
}

