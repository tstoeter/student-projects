import java.util.regex.*;

public class Syntaxizer
{
	String str;

	public Syntaxizer(String s)
	{
		str = s;
	}

	public void analyze()
	{
		/* syntaxregeln:
		   ausdruck <= zahlen
		   ausdruck <= (ausdruck)
		   ausdruck <= ausdruck°ausdruck
		   ausdruck <= funktion ausdruck
		*/

		Pattern pattern;
		Matcher matcher;
		String newstr;

		while (!str.equals("expr"))
		{
			newstr = str;

			// (expr) => expr
			pattern = Pattern.compile("\\(expr\\)");
			matcher = pattern.matcher(newstr);
			newstr = matcher.replaceAll("expr");

			// expr&expr => expr
			pattern = Pattern.compile("expr&expr");
			matcher = pattern.matcher(newstr);
			newstr = matcher.replaceAll("expr");

			// fnexpr => expr
			pattern = Pattern.compile("fnexpr");
			matcher = pattern.matcher(newstr);
			newstr = matcher.replaceAll("expr");

			if (newstr.equals(str))
				throw new IllegalArgumentException("bad syntax: " + str);
			else
				str = newstr;
		}
	}
}

