package example;

import java.io.IOException;
import java.util.Arrays;

import xcsf.MatchSet;
import xcsf.Population;
import xcsf.StateDescriptor;
import xcsf.XCSF;
import xcsf.XCSFConstants;
import xcsf.XCSFUtils;
import xcsf.visualization.ConditionVis;
import xcsf.visualization.PerformanceVis;
import xcsf.visualization.PredictionVis;
import functions.Function;

/**
 * This simple class contains a complete xcsf usecase, i.e. the code to learn a
 * function and afterwards exploit/test the population.
 * 
 * @see ExampleFunction
 * @author Patrick O. Stalph
 */
public class Example {

	/**
	 * Example main method starts a small xcsf run.
	 * <p>
	 * If you want to access the function via the
	 * <code>XCSF.main(String[])</code> using the "xcsf_functions.ini" you must
	 * add the function name to the <code>FunctionFactory</code>.
	 * 
	 * @param args
	 *            not used.
	 */
	public static void main(String[] args) {
		// ---[ create the function ]---
		double scale = 1;
		double modifier = 1;
		double noise = 0;
		Function f = new ExampleFunction(scale, modifier, noise);

		init();

		// ---[ essential code to run xcsf ]---
		System.out.println("----- running xcsf -----");
		XCSF xcsf = new XCSF(f, true);
		Population population = xcsf.singleRun();
		System.out.println("--------- done ---------\r\n\r\n");

		// ---[ exploit the learned classifier population ]---
		exploit(population);
	}

	private static void init() {
		// init constants
		// XCSConstants.load("your_settings.txt");
		XCSFConstants.maxLearningIterations = 30000;
		XCSFConstants.startCompaction = 25000;
		XCSFConstants.maxPopSize = 1000;
		XCSFConstants.doWritePopulation = false;

		// add some listeners
		XCSF.addListener(new PerformanceVis());
		XCSF.addListener(new ConditionVis());
		try {
			XCSF.addListener(new PredictionVis());
		} catch (IllegalStateException e) {
			System.out.println("Gnuplot is not installed/found.");
		} catch (IOException e) {
			System.out.println("Failed to execute GnuPlot.");
			e.printStackTrace();
		}
	}

	private static void exploit(Population population) {
		// use numClosestMatching to ensure matching,
		// even if input is not covered
		boolean numClosestMatching = true;
		MatchSet matchSet = new MatchSet(numClosestMatching);
		for (int i = 0; i < 5; i++) {
			double[] input = { XCSFUtils.Random.uniRand(),
					XCSFUtils.Random.uniRand() };
			double[] prediction = generatePrediction(input, matchSet,
					population);
			System.out.println("input      : " + Arrays.toString(input));
			System.out.println("prediction : " + Arrays.toString(prediction)
					+ "\r\n");
		}
	}

	private static double[] generatePrediction(double[] input,
			MatchSet matchSet, Population population) {
		double[] output = null; // output is not needed for matching/prediction
		StateDescriptor state = new StateDescriptor(input, output);
		// creates the matchset for state
		matchSet.match(state, population);
		double[] prediction = matchSet.getAveragePrediction();
		return prediction;
	}
}
