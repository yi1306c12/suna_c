package functions;

import functions.Function.SimpleFunction;

/**
 * Implements a radial, wave function.
 * 
 * @author Patrick O. Stalph, Martin V. Butz
 */
class RadialSineFunction extends SimpleFunction {

	/**
	 * Default constructor.
	 * 
	 * @param scale
	 * @param modifier
	 * @param noiseDeviation
	 * @param dim
	 */
	RadialSineFunction(double scale, double modifier, double noiseDeviation,
			int dim) {
		super(scale, modifier, noiseDeviation, dim);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see function.Function#evaluate(double[])
	 */
	protected double[] evaluate(double[] input) {
		double[] output = { 0.0 };
		for (double val : input) {
			output[0] += (val - .5) * (val - .5);
		}
		output[0] = Math.exp(-16. * output[0] / input.length)
				* Math.cos(2.0 * Math.PI * output[0] * super.modifier);
		return output;
	}

}
