import java.util.*;
import java.util.regex.*;

public class Processor
{
	LinkedList<String> symtab;
	LinkedList<Double> valtab;
	double value;

	public Processor()
	{
		symtab = new LinkedList<String>();
		valtab = new LinkedList<Double>();

		symtab.add(new String("pi"));
		valtab.add(Double.valueOf(3.1415926535897932384626433832795028841971693993751));

		symtab.add(new String("e"));
		valtab.add(Double.valueOf(2.7182818284590452353602874713526624977572470936999));

		value = 0;
	}

	public int setVariable(String name, double val)
	{
		Pattern pattern = Pattern.compile("[a-z]+");
		Matcher matcher = pattern.matcher(name);

		if (!matcher.matches())
			throw new IllegalArgumentException("bad variable name: non-alphabetic character contained");

		if (FunctionStrings.isFunctionString(name))
			throw new IllegalArgumentException("bad variable name: reserved word");

		// index der variable suchen
		int index = Tokenizer.findInSymTab(symtab, name);

		if (index < 0)	// index/variable existiert nicht
		{
			// also variablebezeichnung in symtab aufnehmen und wert in valtab
			symtab.add(name);
			valtab.add(val);

			index = Tokenizer.findInSymTab(symtab, name);
		}
		else	// variable existiert bereits, also nur wert aktualisieren 
			valtab.set(index, val);

		return index;
	}

	// optimierte setVariable, wenn index bekannt
	public void setVariable(int index, double val)
	{
		valtab.set(index, val);
	}

	public double getVariable(String name)
	{
		double r = 0;

		// index der variablen suchen
		int index = Tokenizer.findInSymTab(symtab, name);

		if (index < 0)	// index/variable existiert nicht
		{
			throw new IllegalArgumentException("bad variable name: variable does not exist");
		}
		else
			r = valtab.get(index);

		return r;
	}

	public Tokenizer process(String str)
	{
	//	boolean b = false;

		// am gleichheitszeichen splitten, um variablen zuweisungen zu erkennen
		String[] s = str.split("=");
		Tokenizer tk;
		Interpreter it;

		if (s.length == 1)	// nur ein split-string, d.h. keine zuweisung, sondern ergebnis des ausdrucks ausgeben
		{
			tk = new Tokenizer(s[0].trim(), symtab);
			tk.tokenize();

			Syntaxizer st = new Syntaxizer(tk.getSyntaxString());
			st.analyze();

			it = new Interpreter(tk.getTokens(), valtab);
			value = it.evaluate();

		//	b = true;
		}
		else if (s.length == 2)	// zwei split-strings, d.h. zuweisung
		{
			tk = new Tokenizer(s[1].trim(), symtab);
			tk.tokenize();

			Syntaxizer st = new Syntaxizer(tk.getSyntaxString());
			st.analyze();

			it = new Interpreter(tk.getTokens(), valtab);

			double d = it.evaluate();
			setVariable(s[0].trim(), d);
			value = d;	// wert nur aktualisieren, wenn setVariable fehlerfrei war
		}
		else	// mehrere gleichheitszeichen sind eine falsche eingabe
			throw new IllegalArgumentException("bad expression: two or more '=' found");

		// return interpreter for optimizing next processing
		return tk;
	}

	public void process(Tokenizer tk)
	{
		Interpreter it = new Interpreter(tk.getTokens(), valtab);
		value = it.evaluate();
	}

	public double getValue()
	{
		return value;
	}
}

