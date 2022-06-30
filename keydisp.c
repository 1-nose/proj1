include<stdio.h>
extern void lcd();
extern void timer();
extern void gpio();
int main()
{
	unsigned int before;
	before=gpio();
	timer();
	lcd();
}
