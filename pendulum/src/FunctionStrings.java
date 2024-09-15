class FunctionStrings
{
	public static final String[] fstr = {	"exp",
						"ln",
						"lg",
						"log",
						"ld",
						"sqrt",
						"cbrt",
						"sin",
						"cos",
						"tan",
						"cot",
						"asin",
						"acos",
						"atan",
						"sinh",
						"cosh",
						"tanh",
						"abs",
						"sign",
						"ceil",
						"floor",
						"round"	};

	public static boolean isFunctionString(String str)
	{
		boolean r = false;

		// alle möglichen funktionsnamen durchtesten
		for (int i=0; i<FunctionStrings.fstr.length; i++)
		{
			if (str.equals(FunctionStrings.fstr[i]))
			{
				r = true;
				break;
			}
		}

		return r;
	}
}

