
import mgr
import random

c1 = 0.00
c2 = 0.50
c3 = 0.99
c1_change = 0.01
c2_change = 0.02
c3_change = 0.04
c3_mult = 1
c2_mult = 1
c1_mult = 1

while True:
	
	# ChangeColor has to compete for mutexes with the GetColor() function that is used extensively in the
	# render loop. because of this, excessive use of this function(like here in an infinite loop)
	# will cause lag 
	
	mgr.ChangeColor('BACKGROUND_COLOR', c1, c2, c3)
	mgr.ChangeColor('BUTTON_COLOR', c2, c3, c1*0.2)
	mgr.ChangeColor('BUTTON_TEXTCOLOR', c2, c1, c3)
	mgr.ChangeColor('TEXT_COLOR', c2, c3, c3)
	mgr.ChangeColor('HEXDUMP_BACKGROUND_COLOR', c2, c1, c1)
	mgr.ChangeColor('HEXDUMP_BUTTONCOLOR', c3, c1, c2)
	mgr.ChangeColor('HEXDUMP_TEXTCOLOR', c2, c2, c3)
	mgr.ChangeColor('PEPARSER_BUTTONCOLOR', c1, c3, c1)
	mgr.ChangeColor('PEPARSER_TEXTCOLOR', c2, c1, c2)
	mgr.ChangeColor('PEPARSER_TEXTCOLOR', (c2+0.1)*0.5, (c1+0.1)*0.5, c2)
	mgr.ChangeColor('PEPARSER_COLHEADER_COLOR', (c2+0.1)*0.5, (c2+0.1)*0.5, c2)
	
	c1_change = random.uniform(0.01, 0.04)
	c2_change = random.uniform(0.01, 0.02)
	c3_change = random.uniform(0.01, 0.01)


	if c1 + c1_change < 0.00 or c1 + c1_change > 1.00:
		c1_mult = c1_mult * -1
	if c2 + c2_change < 0.00 or c2 + c2_change > 1.00:
		c2_mult = c2_mult * -1
	if c3 + c3_change < 0.00 or c3 + c3_change > 1.00:
		c3_mult = c3_mult * -1
	
	c1 = c1 + (c1_change * c1_mult)
	c2 = c2 + (c2_change * c2_mult)
	c3 = c3 + (c3_change * c3_mult)