package functions;

import xcsf.StateDescriptor;
import xcsf.XCSFUtils;

/**
 * This is the general function interface. If you want to implement your own
 * function, take a look at the two convenience classes
 * <code>SimpleFunction</code> and <code>CompositeFunction</code>.
 * <p>
 * You can add your function to the <code>FunctionFactory</code> to access it
 * via the "xcsf_functions.ini".
 * 
 * @author Patrick O. Stalph, Martin V. Butz
 */
public interface Function {

	/**
	 * Returns the next problem instance for XCSF to learn. For theoretical
	 * problems this will usually be a randomly generated input and the
	 * corresponding scaled (and possibly noisy) output.
	 * 
	 * @return The next problem instance
	 * @see StateDescriptor
	 */
	public StateDescriptor nextProblemInstance();

	/**
	 * Returns the function value(s) as a double vector without noise. The
	 * function value refers to the last problem input instance generated.
	 * 
	 * @return The noiseless function value(s)
	 * @throws IllegalArgumentException
	 *             if no problem instance is generated, yet.
	 */
	public double[] noiselessFunctionValue();

	/**
	 * Returns the condition input dimension.
	 * 
	 * @return the dimension of the condition input space.
	 */
	public int getConditionInputDimension();

	/**
	 * Returns the prediction input dimension.
	 * 
	 * @return the dimension of the prediction input space.
	 */
	public int getPredictionInputDimension();

	/**
	 * Returns the function output dimension.
	 * 
	 * @return the dimension of the function output space.
	 */
	public int getOutputDimension();

	/**
	 * * This class encapsulates methods for simple testfunctions with
	 * 1-dimensional output. Extending classes need to implement the
	 * {@link #evaluate(double[])} method to define the function.
	 * <p>
	 * If your function evaluation is noisy for any reason, set the
	 * noiseDeviation to zero. Note that the {@link #noiselessFunctionValue()}
	 * is used for performance evaluation only.
	 * 
	 * @author Patrick Stalph
	 */
	public abstract class SimpleFunction implements Function {

		protected double scale;
		protected double noiseDeviation;
		protected double modifier;
		protected int dim;

		protected StateDescriptor currentState;

		/**
		 * Default constructor.
		 * 
		 * @param scale
		 *            the function scale.
		 * @param modifier
		 *            the function modifier.
		 * @param noiseDeviation
		 *            the (gaussian) noise deviation.
		 * @param dim
		 *            the input dimension of the function.
		 */
		public SimpleFunction(double scale, double modifier,
				double noiseDeviation, int dim) {
			this.scale = scale;
			this.noiseDeviation = noiseDeviation;
			this.modifier = modifier;
			this.dim = dim;
		}

		/*
		 * (non-Javadoc)
		 * 
		 * @see function.Function#getNextProblemInstance()
		 */
		public StateDescriptor nextProblemInstance() {
			// generate random input
			double[] input = new double[this.dim];
			for (int i = 0; i < input.length; i++) {
				input[i] = XCSFUtils.Random.uniRand();
			}
			// calculate output
			double[] output = evaluate(input);
			if (this.scale != 1 || this.noiseDeviation != 0) {
				// scale and add noise
				for (int i = 0; i < output.length; i++) {
					output[i] *= this.scale;
					output[i] += this.noiseDeviation
							* XCSFUtils.Random.normRand();
				}
			}
			// set state and return
			this.currentState = new StateDescriptor(input, output);
			return this.currentState;
		}

		/*
		 * (non-Javadoc)
		 * 
		 * @see function.Function#getCurrentNoiselessFunctionValue()
		 */
		public double[] noiselessFunctionValue() {
			if (this.currentState != null) {
				if (this.noiseDeviation == 0) {
					return this.currentState.getOutput();
				}
				double[] noiseless = evaluate(this.currentState
						.getConditionInput());
				for (int i = 0; i < noiseless.length; i++) {
					noiseless[i] *= this.scale;
				}
				return noiseless;
			} else {
				throw new IllegalStateException(
						"No problem instance generated!");
			}
		}

		/*
		 * (non-Javadoc)
		 * 
		 * @see function.Function#getConditionInputDimension()
		 */
		public int getConditionInputDimension() {
			return this.dim;
		}

		/*
		 * (non-Javadoc)
		 * 
		 * @see function.Function#getPredictionInputDimension()
		 */
		public int getPredictionInputDimension() {
			return this.dim;
		}

		/*
		 * (non-Javadoc)
		 * 
		 * @see function.Function#getOutputDimension()
		 */
		public int getOutputDimension() {
			return 1;
		}

		/**
		 * Evaluates this function using the given input.
		 * 
		 * @param input
		 *            the input for this function.
		 * @return the output for this function using the given
		 *         <code>input</code>.
		 */
		protected abstract double[] evaluate(double[] input);
	}
}
