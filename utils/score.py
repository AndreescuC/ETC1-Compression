import string
import io
import numpy as np
import sys

def get_score( isGPU, avgRMSE, avgTIME ):
	print "AVG (isgpu, rmse, time) : ", isGPU, avgRMSE, avgTIME

	score = 0

	if isGPU:
		score += 10

	if avgRMSE < 45:
		score += 10
	if avgRMSE < 20:
		score += 10
	if avgRMSE < 6:
		score += 10

	if avgTIME < 5000:
		score += 10
	if avgTIME < 2000:
		score += 10
	if avgTIME < 1000:
		score += 10

	# if quality is awfull, speed doesn't count 
	if avgRMSE > 100:
		score = 0

	print "SCORE: ", score, "/ 70"

def main():
	isGPU = False
	listRMSE = []
	listTIME = []

	if len( sys.argv ) > 1:
		if sys.argv[1].find("results") < 0:
			print( "Error invalid arguments" )
			sys.exit(1)
	else:
		print( "Error no arguments to python scrypt" )
		sys.exit(1)

	try:
		fileObj = open( sys.argv[1], "r" )
		fileLines = fileObj.read().split( "\n" )
 
		for line in fileLines:
			if line.find( "GPU = YES" ) >= 0:
				isGPU = True	
			if line.find("RMSE") > 0:		
				listRMSE.append( float( line.split( "RMSE =" )[1] ) )
			if line.find( "TIME" ) > 0:
				listTIME.append( float( line.split( "TIME.ms =" )[1] ) )
		fileObj.close()
	except IOError:
		print( "Error: Could not find results.txt" )
	except EOFError:
		print ( "End of file" )

	if len( listRMSE ) == 0:
		print( "Error: Could not find any quality results" )
		listRMSE = [ 99999999 ]

	npRMSE = np.array( listRMSE )
	npTIME = np.array( listTIME )
	avgRMSE = np.average( npRMSE )
	avgTIME = np.average( npTIME )
	
	# compute score
	get_score( isGPU, avgRMSE, avgTIME )

if __name__ == '__main__':
	main()
