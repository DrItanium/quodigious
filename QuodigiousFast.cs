using System;
using System.Reflection;
using System.Runtime.Serialization;
using System.Text;
using System.Collections.Generic;
using System.Runtime;
using System.Linq;
using System.Text.RegularExpressions;

public class Program
{
	public static void Main(string[] args)
	{
		Regex r = new Regex(" ");
		foreach(string input in r.Split(Console.ReadLine())) {
			int len = int.Parse(input);
			for(long l = (long)(2.2 * Math.Pow(10, len - 1)); l < (long)(Math.Pow(10, len)); l++) {
				if (IsQuodigious(l, len)) {
					Console.WriteLine(l);
				}
			}
			Console.WriteLine();
		}
	}
	public static bool IsQuodigious(long value, int length)
	{
		long current = value;
		int sum = 0,
				prod = 1;
		for(int i = 0; i < length; i++) {
			int result = (int)(current % 10L);
			if(result < 2) {
				return false;
			}
			sum += result;
			prod *= result;
			current /= 10L;
		}
		return (value % sum == 0) && (value % prod == 0); 
	}

}
