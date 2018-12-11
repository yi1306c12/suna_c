package functions;

import functions.Function.SimpleFunction;

/**
 * Implements Schaal&Atkeson's (1998) ridge function.
 * 
 * @author Patrick O. Stalph, Martin V. Butz
 */
class CrossedRidgeFunction extends SimpleFunction {

	/**
	 * Default constructor.
	 * 
	 * @param scale
	 * @param noiseDeviation
	 */
	CrossedRidgeFunction(double scale, double noiseDeviation) {
		super(scale, 0, noiseDeviation, 2);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see function.Function#evaluate(double[])
	 */
	protected double[] evaluate(double[] input) {
		double[] output = new double[1];
		double valX = input[0] * 2. - 1.;
		double valY = input[1] * 2. - 1.;
		double sq1 = valX * valX;
		double sq2 = valY * valY;
		output[0] = Math.exp(-10. * sq1);
		double res2 = Math.exp(-50. * sq2);
		double res3 = 1.25 * Math.exp(-5. * (sq1 + sq2));
		if (res2 > output[0]) {
			output[0] = res2;
		}
		if (res3 > output[0]) {
			output[0] = res3;
		}
		return output;
	}
}
