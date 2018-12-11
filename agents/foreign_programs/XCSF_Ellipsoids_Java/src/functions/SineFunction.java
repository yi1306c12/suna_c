package functions;

import functions.Function.SimpleFunction;

/**
 * Implements a sine function (sine of sum for every dimension).
 * 
 * @author Patrick O. Stalph, Martin V. Butz
 */
class SineFunction extends SimpleFunction {

	/**
	 * Default constructor.
	 * 
	 * @param scale
	 * @param modifier
	 * @param noiseDeviation
	 * @param dim
	 */
	SineFunction(double scale, double modifier, double noiseDeviation, int dim) {
		super(scale, modifier, noiseDeviation, dim);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see functions.Function.SimpleFunction#evaluate(double[])
	 */
	protected double[] evaluate(double[] input) {
		double sum = 0.0;
		for (int i = 0; i < input.length; i++) {
			sum += input[i];
		}
		return new double[] { Math.sin(super.modifier * Math.PI * sum) };
	}
}
