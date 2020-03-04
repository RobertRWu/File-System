#include "include/utility.h"

int Utility::Min(int a, int b) 
{
	if (a < b)
		return a;

	return b;
}

/* 
 * Function: Judge if a string only includes number.
 * Parameters£º
 *     str: The string to be judged.
 * Returns:
 *     boolean, if the string only includes number, return 1
 */
bool Utility::IsInt(string str)
{
	stringstream ss(str);
	int i;
	char c;

	/* transform string into int, if succeed, return non-zero */
	if (!(ss >> i)) 
		return false;
	
	/* Check the format like 1.a, stringstream will transform the rear part of string 
	 * into char, if succeed, means there exists at least a letter, return non-zero */
	if (ss >> c)
		return false;

	return true;
}

bool Utility::IsDouble(string str)
{
	stringstream ss(str);
	double d;
	char c;

	/* transform string into double, if succeed, return non-zero */
	if (!(ss >> d))
		return false;

	/* Check the format like 1.a, stringstream will transform the rear part of string
	 * into char, if succeed, means there exists at least a letter, return non-zero */
	if (ss >> c)
		return false;

	return true;
}