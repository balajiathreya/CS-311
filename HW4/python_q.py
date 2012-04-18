#!/usr/bin/python 
import sys
import os

def k_number(big_num, product = 0):
	for i in range(len(str(big_num))-4):product = max(reduce(lambda x, y: x*y, [int(str(big_num)[f]) for f in range(i,i+5)]), product)
	return product

def seive():
	numbers = range(2, 8000)
	for i in numbers: numbers = filter(lambda x: x == i or x % i, numbers)
	print numbers[1001 - 1]	
	
def ups_phone_numbers():
    pattern = re.compile(r'''(\d{3})\D*(\d{3})\D*(\d{4})''', re.VERBOSE)

    phone_numbers = urllib.urlopen('http://www.ups.com/content/us/en/contact/index.html?WT.svl=SecNav').readlines()

    for number in phone_numbers:
        for match in pattern.finditer(number):
            if match:
                   code, exchange, terminal = match.groups()
                   print code, exchange, terminal

def sum_of_multiples():
	print sum([no for no in range(1000) if no % 3 == 0 or no % 5 == 0])

def term_and_course():	
	t = raw_input("What is the term: ")
	c = raw_input("What is the course: ")
	if ((t == '') | (c == '')):
		print "Error: Both term and course must be defined."
	elif (os.path.exists(term) == True:
		print "Error: Directory already exists."
	else:
		make_dirs(term, course)

def main():
	var = -1
	while var != 5:
		os.system('clear')
		print '''Options : 
0 Directories
1 phone numbers.
2 prime.
3 product of five consecutive digits in the 1000-digit number.
4 sum of all the multiples of 3 or 5 below 1000.
5 Quit'''
		print ''
		var = int(raw_input("Choice: "))
		print ''
		if var == 0:
			term_and_course()
		elif var == 1:
			ups_phone_numbers()
		elif var == 2:
			seive()
		elif var == 3:
			print "ans is " + str((k_number(7316717653133062491922511967442657474235534919493496983520312774506326239578318016984801869478851843858615607891129494954595017379583319528532088055111254069874715852386305071569329096329522744304355766896648950445244523161731856403098711121722383113622298934233803081353362766142828064444866452387493035890729629049156044077239071381051585930796086670172427121883998797908792274921901699720888093776657273330010533678812202354218097512545405947522435258490771167055601360483958644670632441572215539753697817977846174064955149290862569321978468622482839722413756570560574902614079729686524145351004748216637048440319989000889524345065854122758866688116427171479924442928230863465674813919123162824586178664583591245665294765456828489128831426076900422421902267105562632111110937054421750694165896040807198403850962455444362981230987879927244284909188845801561660979191338754992005240636899125607176060588611646710940507754100225698315520005593572972571636269561882670428252483600823257530420752963450)))
		elif var == 4:
			sum_of_multiples()
		elif var == 5:
			break
		else:
			print "Error: Invalid choice."
		print ''
		raw_input("Press any key to continue...")
	
if __name__ == "__main__":
	main()

