package xcsf;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Properties;
import java.util.Vector;

import xcsf.XCSFUtils.GnuPlotConsole;
import xcsf.classifier.Classifier;
import xcsf.visualization.ConditionVis;
import xcsf.visualization.PerformanceVis;
import xcsf.visualization.PredictionVis;
import functions.Function;
import functions.FunctionFactory;

/**
 * The main XCSF class coordinates the learning process and listeners. The
 * {@link #main(String[])} method loads the <code>XCSConstants</code>,
 * functions, Visualization, creates the xcsf-instance and evaluates the
 * functions.
 * 
 * @author Patrick O. Stalph, Martin V. Butz
 */
public class XCSF {

	/**
	 * This filename is used to temporarily store the prediction approximation.
	 * 
	 * @see PredictionVis
	 */
	public final static String APPROXIMATION_FILE = "approximation.temp";

	final static String LINE_SEPERATOR = System.getProperty("line.separator");

	// xcsf ini & output files
	private final static String SETTINGS_FILE = "xcsf.ini";
	private final static String FUNCTION_FILE = "xcsf_functions.ini";
	private final static String VISUALIZATION_FILE = "xcsf_visualization.ini";

	// used for all runs
	private static Vector<XCSFListener> listener = new Vector<XCSFListener>();
	private Function function;
	private PerformanceEvaluator performanceEvaluator;

	// re-created for every run
	private Population population;
	private MatchSet matchSet;
	private EvolutionaryComp evolutionaryComponent;

	/**
	 * Main method loads the <code>XCSConstants</code>, initializes functions,
	 * registers listeners and starts XCSF. This method is used for the
	 * executable JAR package, too.
	 * 
	 * @param args
	 *            not used.
	 */
	public static void main(String[] args) {
		// load xcsf constants, functions, visualization
		Function[] functions;
		try {
			functions = initialize();
		} catch (Exception e) {
			System.err.println("Message: " + e.getMessage() + "\r\n");
			e.printStackTrace();
			return; // abort
		}

		// ---[ manual settings ]----------------------------------------------
		// XCSConstants.maxLearningIterations = 500000;
		// XCSConstants.startCompaction = (int) (0.9 *
		// XCSConstants.maxLearningIterations);
		// XCSConstants.resetRLSPredictionsAfterSteps =
		// XCSConstants.startCompaction;
		// XCSConstants.maxPopSize = 1600;
		// XCSConstants.numberOfExperiments = 1;
		// XCSConstants.minConditionStretch = 0.01;
		// XCSConstants.coverConditionRange = 0.5;
		// XCSConstants.doWritePopulation = false;
		// XCSConstants.outputFolder = "/home/pat/Desktop/output";
		// XCSConstants.resetRLSPredictionsAfterSteps = -1;

		// ConditionVis.slowMotion = true;
		// ConditionVis.visualizationDelay = 1000;
		// ConditionVis.visualizedConditionSize = 1.0f;

		// functions = new Function[] { FunctionFactory.getFunction("sine", 1,
		// 4,
		// 0, 2) };
		// listener.clear();

		// ---[ run xcsf for every function ]----------------------------------
		for (Function f : functions) {
			Population finalPopulation = null;
			boolean verbose = XCSFConstants.numberOfExperiments == 1;
			XCSF xcsf = new XCSF(f, verbose);
			// run several single experiments
			for (int i = 1; i <= XCSFConstants.numberOfExperiments; i++) {
				System.out.println("> Experiment " + i + "/"
						+ XCSFConstants.numberOfExperiments + " "
						+ f.getClass().getSimpleName());
				long time = System.currentTimeMillis();
				finalPopulation = xcsf.singleRun();
				time = (System.currentTimeMillis() - time) / 1000;
				System.out.println("> Experiment " + i + "/"
						+ XCSFConstants.numberOfExperiments + " done in "
						+ (int) (time / 60) + "m " + (time % 60) + "s");

				// write classifier population, if wanted.
				if (XCSFConstants.doWritePopulation) {
					String filename = XCSFConstants.outputFolder
							+ File.separator + f.getClass().getSimpleName()
							+ "-pop" + (i < 10 ? "0" + i : i) + ".txt";
					try {
						FileWriter fw = new FileWriter(filename);
						for (Classifier cl : finalPopulation) {
							fw.write(cl.toString() + LINE_SEPERATOR);
						}
						fw.flush();
						fw.close();
					} catch (IOException e) {
						System.err.println("Failed to write population to '"
								+ filename + "'.");
					}
				}
			}

			// store performance, prediction plot and population screenshot
			if (XCSFConstants.doWriteOutput) {
				String basename = XCSFConstants.outputFolder + File.separator
						+ f.getClass().getSimpleName() + "-";
				String performanceFilename = basename + "avgPerformance";
				String screenshotFilename = basename + "population";
				String predictionFilename = basename + "prediction";
				xcsf.performanceEvaluator
						.writeAvgPerformance(performanceFilename);
				ConditionVis.screenshot(finalPopulation.shallowCopy(),
						screenshotFilename);
				PredictionVis.epsPlot(finalPopulation.shallowCopy(),
						predictionFilename, f.getClass().getSimpleName());
			}
		}

		// store settings
		if (XCSFConstants.doWriteOutput) {
			// settings
			File source = new File(SETTINGS_FILE);
			File dest = new File(XCSFConstants.outputFolder + File.separator
					+ SETTINGS_FILE);
			XCSFUtils.fileCopy(source, dest);
			// function
			source = new File(FUNCTION_FILE);
			dest = new File(XCSFConstants.outputFolder + File.separator
					+ FUNCTION_FILE);
			XCSFUtils.fileCopy(source, dest);
		}
	}

	/**
	 * Registers the given listener for static access from all <code>XCSF</code>
	 * instances.
	 * <p>
	 * Note: Don't run two or more xcsf instances in parallel with listeners :-)
	 * 
	 * @param l
	 *            The listener to register.
	 */
	public static void addListener(XCSFListener l) {
		listener.add(l);
	}

	/**
	 * Removes the given listener (object reference).
	 * 
	 * @param l
	 *            The listener to remove.
	 */
	public static void removeListener(XCSFListener l) {
		listener.remove(l);
	}

	/**
	 * Loads the <code>XCSFConstants</code>, the <code>Function</code> array to
	 * evaluate, the default visualization and initializes the output directory,
	 * if necessary.
	 * 
	 * @return the functions to evaluate.
	 * @throws Exception
	 *             if the functions could not be loaded.
	 */
	private static Function[] initialize() throws Exception {
		// load xcsf-constants
		System.out.print("loading xcsf constants...               ");
		try {
			XCSFConstants.load(SETTINGS_FILE);
			System.out.println("done");
		} catch (Exception e) {
			System.err.println("Failed to load '" + SETTINGS_FILE + "'");
			throw e;
		}
		// load functions
		System.out.print("loading functions...                    ");
		Function[] functions = null;
		try {
			functions = FunctionFactory.loadFromPropertiesFile(FUNCTION_FILE);
			System.out.println("done  " + functions.length + " function(s)");
			for (Function f : functions) {
				System.out.println(" * " + f.getClass().getSimpleName());
			}
		} catch (Exception e) {
			System.err.println("Failed to load '" + FUNCTION_FILE + "'");
			throw e;
		}
		// load default visualization listeners
		System.out.print("loading visualization...                ");
		try {
			loadDefaultListeners(functions[0].getConditionInputDimension());
			System.out.println("done  " + listener.size()
					+ " listener(s) registered");
			for (XCSFListener l : listener) {
				System.out.println(" * " + l.getClass().getSimpleName());
			}
		} catch (IOException e) {
			System.err.println("Failed to load '" + VISUALIZATION_FILE + "'");
			// ignore and go on
		}
		// init output directory
		if (XCSFConstants.doWriteOutput || XCSFConstants.doWritePopulation) {
			File outputDirectory = new File(XCSFConstants.outputFolder);
			System.out.print("initializing output directory...        ");
			if (!outputDirectory.exists() && !outputDirectory.mkdir()) {
				System.err.println("Failed to create output directory: '"
						+ XCSFConstants.outputFolder + "'");
				throw new IOException("Could not create directory: "
						+ outputDirectory.getAbsolutePath());
			}
			XCSFConstants.outputFolder = outputDirectory.getAbsolutePath();
			System.out.println("done");
			System.out.println(" * " + outputDirectory.getName());
		}
		// initial seed for random number generator
		System.out.print("initializing random number generator... ");
		if (XCSFConstants.doRandomize) {
			XCSFUtils.Random
					.setSeed(11 + (System.currentTimeMillis()) % 100000);
		} else {
			XCSFUtils.Random.setSeed(XCSFConstants.initialSeed);
		}
		System.out.println("done  " + (XCSFConstants.doRandomize ? "rnd-" : "")
				+ "seed(" + XCSFConstants.initialSeed + ")");
		System.out.println();
		return functions;
	}

	/**
	 * Reads the <code>XCSF.VISUALIZATION_FILE</code> and registers the
	 * appropriate listeners, if the doVisualization flag is set.
	 * 
	 * @param dimension
	 *            the function input space dimension.
	 * @throws IOException
	 *             if the file could not be loaded.
	 */
	private static void loadDefaultListeners(int dimension) throws IOException {
		FileInputStream in = new FileInputStream(VISUALIZATION_FILE);
		Properties p = new Properties();
		p.load(in);

		boolean doVisualization = Boolean.parseBoolean(p
				.getProperty("doVisualization"));
		String gnuplotExe = p.getProperty("gnuplotExecutable");
		if (!gnuplotExe.toLowerCase().equals("default")) {
			XCSFUtils.GnuPlotConsole.gnuPlotExecutable = gnuplotExe;
		}
		if (doVisualization) {
			ConditionVis.slowMotion = Boolean.parseBoolean(p
					.getProperty("slowMotion"));
			ConditionVis.visualizationSteps = Integer.parseInt(p
					.getProperty("updateVisualizationSteps"));
			ConditionVis.visualizedConditionSize = Float.parseFloat(p
					.getProperty("relativeVisualizedConditionSize"));
			ConditionVis.visualizationTransparency = Float.parseFloat(p
					.getProperty("visualizationTransparencyDegree"));
			ConditionVis.visualizationDelay = Integer.parseInt(p
					.getProperty("visualizationDelay"));
			/*
			 * Add the listeners. PerformanceVis for all dimensions, no
			 * preconditions. ConditionVis for 2D (always) and 3D, if Java3D is
			 * installed. PredictionVis for 2D, if GnuPlot is installed.
			 */
			listener.add(new PerformanceVis());
			if (dimension == 2) {
				listener.add(new ConditionVis());
				if (GnuPlotConsole.gnuPlotExecutable == null
						|| !GnuPlotConsole.gnuPlotExecutable.toLowerCase()
								.equals(GnuPlotConsole.NOT_INSTALLED)) {
					try {
						listener.add(new PredictionVis());
					} catch (IllegalStateException e) {
						System.err.print("GnuPlot is not installed! ");
					} catch (IOException e) {
						System.err.print("Failed to execute GnuPlot! ");
					}
				}
			} else if (dimension == 3) {
				// check for Java3D
				try {
					XCSF.class.getClassLoader().loadClass(
							"com.sun.j3d.utils.universe.SimpleUniverse");
					listener.add(new ConditionVis());
				} catch (ClassNotFoundException e) {
					System.err.print("Java3D is not installed! ");
				}
			}
		}
		in.close();
	}

	/**
	 * XCSF constructor creates the xcsf instance using the given
	 * <code>function</code>. The <code>verbose</code> flag indicates, if the
	 * performance should be printed to <code>System.out</code>.
	 * 
	 * @param function
	 *            the function to be learned.
	 * @param verbose
	 *            determines if the performance should be printed to
	 *            <code>System.out</code>.
	 */
	public XCSF(Function function, boolean verbose) {
		this.function = function;
		this.performanceEvaluator = new PerformanceEvaluator(verbose);
	}

	/**
	 * This method starts one experiment for
	 * {@link XCSFConstants#maxLearningIterations} and returns the resulting
	 * <code>Population</code>.
	 * 
	 * @return the final population.
	 */
	public Population singleRun() {
		this.population = new Population();
		this.matchSet = new MatchSet(XCSFConstants.doNumClosestMatch);
		this.evolutionaryComponent = new EvolutionaryComp();
		this.performanceEvaluator.nextExperiment();

		// -----[ main loop ]-----
		for (int iteration = 0; iteration <= XCSFConstants.maxLearningIterations; iteration++) {
			// 1) get next problem instance
			StateDescriptor state = this.function.nextProblemInstance();
			// 2) match & cover, if necessary
			this.matchSet.match(state, this.population);
			this.matchSet.ensureStateCoverage(this.population, iteration);
			// 3) evaluate performance
			double[] functionPrediction = this.matchSet.getAveragePrediction();
			double[] functionValue = this.function.noiselessFunctionValue();
			if (functionValue == null) {
				functionValue = state.getOutput();
			}
			this.performanceEvaluator.evaluate(this.population, this.matchSet,
					iteration, functionValue, functionPrediction);
			// 4) update matching classifiers
			this.matchSet.updateClassifiers();
			// 5) evolution
			this.evolutionaryComponent.evolve(this.population, this.matchSet,
					state, iteration);

			// inform listeners
			this.callListeners(iteration, state);

			// reset rls prediction at next iteration?
			if (iteration + 1 == XCSFConstants.resetRLSPredictionsAfterSteps) {
				for (Classifier cl : this.population) {
					cl.getRlsPrediction().resetGainMatrix();
				}
			}

			// start compaction at next iteration?
			if (iteration + 1 == XCSFConstants.startCompaction) {
				if (XCSFConstants.numberOfExperiments == 1) {
					System.out.println("> compaction phase");
				}
				this.evolutionaryComponent.setCondensation(true);
				if (XCSFConstants.compactionType % 2 == 1) {
					// type 1 & 3
					this.matchSet.setNumClosestMatching(true);
				}
				if (XCSFConstants.compactionType >= 2) {
					// type 2, 3
					this.population.applyGreedyCompaction();
				}
			}
		} // ---[ end loop ]------
		return this.population;
	}

	/**
	 * Informs all listeners about the changes.
	 * 
	 * @param iteration
	 *            the current iteration.
	 * @param state
	 *            the current state.
	 */
	private void callListeners(int iteration, StateDescriptor state) {
		if (listener.isEmpty()) {
			return;
		}
		// prevent side effects from missbehaving listeners :)
		Classifier[] populationCopy = this.population.shallowCopy();
		Classifier[] matchsetCopy = this.matchSet.shallowCopy();
		double[][] performance = this.performanceEvaluator
				.getCurrentExperimentPerformance();
		for (XCSFListener l : listener) {
			l.stateChanged(iteration, populationCopy, matchsetCopy, state,
					performance);
		}
	}
}
