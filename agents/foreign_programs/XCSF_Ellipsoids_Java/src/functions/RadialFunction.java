package functions;

import functions.Function.SimpleFunction;

/**
 * Implements a radial, Gaussian-like function.
 * 
 * @author Patrick O. Stalph, Martin V. Butz
 */
class RadialFunction extends SimpleFunction {

	/**
	 * Default constructor.
	 * 
	 * @param scale
	 * @param modifier
	 * @param noiseDeviation
	 * @param inputDimension
	 */
	RadialFunction(double scale, double modifier, double noiseDeviation,
			int inputDimension) {
		super(scale, modifier, noiseDeviation, inputDimension);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see function.Function#evaluate(double[])
	 */
	protected double[] evaluate(double[] input) {
		// exp(-1 * sum_i (x_i-0.5)^2 * mod)
		double[] output = { 0.0 };
		for (double val : input) {
			output[0] += (val - 0.5) * (val - 0.5);
		}
		output[0] = Math.exp(-1. * output[0] * super.modifier);
		return output;
	}

}
