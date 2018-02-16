/*
 * G_mem.h
 *
 *  Created on: 08.09.2010
 *      Author: stephaniebayer
 *
 *      An instance of this group represent the value of a group member
 *
 */

#ifndef G_MEM_H_
#define G_MEM_H_

#include <NTL/ZZ.h>
NTL_CLIENT

class G_mem {
private:
	ZZ val; //Value of the group member
public:
	//Constructors & destructor
	G_mem();
	G_mem(ZZ x);
	G_mem(long x);
	virtual ~G_mem();

	//Access to the variable val
	ZZ value() const;
	//Return the inverse of an element
	G_mem inv();
	//Operators
	G_mem op(const G_mem& x, const G_mem& y);
	bool operator==(const G_mem& y) const;
};



#endif /* G_MEM_H_ */
