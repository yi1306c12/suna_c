package functions;

import functions.Function.SimpleFunction;

/**
 * Implements a sine-in-sine function.
 * 
 * @author Patrick O. Stalph, Martin V. Butz
 */
class SineInSineFunction extends SimpleFunction {

	/**
	 * Default constructor.
	 * 
	 * @param scale
	 * @param modifier
	 * @param noiseDeviation
	 */
	SineInSineFunction(double scale, double modifier, double noiseDeviation) {
		super(scale, modifier, noiseDeviation, 2);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see function.Function#evaluate(double[])
	 */
	protected double[] evaluate(double[] input) {
		double[] output = { 0.0 };
		if (this.dim == 2) {
			// f(x,y) = sin(mod * pi * (x + sin(pi * y)))
			output[0] = Math.sin(super.modifier * Math.PI
					* (input[0] + Math.sin(Math.PI * input[1])));
		} else {
			throw new IllegalStateException("Bad dimension - not implemented.");
		}
		return output;
	}
}
