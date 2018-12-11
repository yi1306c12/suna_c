package functions;

import functions.Function.SimpleFunction;

/**
 * Another sine function (sum of sine for every dimension).
 * 
 * @author Patrick O. Stalph, Martin V. Butz
 */
class SineFunction2 extends SimpleFunction {

	/**
	 * Default constructor.
	 * 
	 * @param scale
	 * @param modifier
	 * @param noiseDeviation
	 * @param dim
	 */
	SineFunction2(double scale, double modifier, double noiseDeviation, int dim) {
		super(scale, modifier, noiseDeviation, dim);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see function.Function#evaluate(double[])
	 */
	protected double[] evaluate(double[] input) {
		// sin(2 pi x) + sin(2 pi y) + ...
		double output[] = { 0 };
		for (int d = 0; d < this.dim; d++) {
			output[0] += Math.sin(super.modifier * Math.PI * input[d]);
		}
		return output;
	}
}
