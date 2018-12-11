package example;

import functions.Function.SimpleFunction;

/**
 * Simple polynomial example function with 2D input / 1D output.
 * 
 * <pre>
 * f(x, y) = x &circ; 2 + y &circ; 2 - (x * y) &circ; 2
 * </pre>
 * 
 * @author Patrick O. Stalph
 */
public class ExampleFunction extends SimpleFunction {

	/**
	 * This constructor just calls the superconstructor.
	 * 
	 * @param scale
	 * @param modifier
	 * @param noiseDeviation
	 */
	ExampleFunction(double scale, double modifier, double noiseDeviation) {
		super(scale, modifier, noiseDeviation, 2);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see function.Function#evaluate(double[])
	 */
	protected double[] evaluate(double[] input) {
		double sqX = input[0] * input[0];
		double sqY = input[1] * input[1];
		double functionValue = sqX + sqY - sqX * sqY;
		// 1D output
		double[] output = { functionValue };
		return output;
	}
}
