package xcsf;

import java.util.Iterator;

import xcsf.classifier.Classifier;

/**
 * The MatchSet class extends <code>ClassifierSet</code> and encapsulates
 * <code>ClassifierSet</code> methods used for matchset creation, covering and
 * update.
 * 
 * @author Patrick O. Stalph, Martin V. Butz
 */
public class MatchSet extends ClassifierSet {

	private StateDescriptor state;
	private boolean numClosestMatching;

	/**
	 * Default constructor.
	 * 
	 * @param doNumClosestMatch
	 *            true, if num-closest-matching should be active.
	 */
	public MatchSet(boolean doNumClosestMatch) {
		super();
		this.numClosestMatching = doNumClosestMatch;
	}

	/**
	 * Creates the matchset for the <code>currentState</code>.
	 * 
	 * @param currentState
	 *            the state to match.
	 * @param population
	 *            the population.
	 */
	public void match(StateDescriptor currentState, Population population) {
		this.clear();
		this.state = currentState;
		this.addMatchingClassifiers(population);
	}

	/**
	 * Calculates the average prediction weighted by fitness.
	 * 
	 * @return The average prediction of all classifiers in this matchSet.
	 */
	public double[] getAveragePrediction() {
		Iterator<Classifier> it = this.iterator();
		Classifier cl = it.next();
		double[] prediction = cl.predict(this.state);
		int n = prediction.length;
		double fitness = cl.getFitness();
		double fitnessSum = fitness;
		double[] avgPrediction = new double[n];
		for (int i = 0; i < n; i++) {
			avgPrediction[i] = prediction[i] * fitness;
		}
		while (it.hasNext()) {
			cl = it.next();
			prediction = cl.predict(this.state);
			fitness = cl.getFitness();
			fitnessSum += fitness;
			for (int i = 0; i < n; i++) {
				avgPrediction[i] += prediction[i] * fitness;
			}
		}
		for (int i = 0; i < n; i++) {
			avgPrediction[i] /= fitnessSum;
		}
		return avgPrediction;
	}

	/**
	 * Sets the numClosestMatching flag.
	 * 
	 * @param value
	 *            the value to set.
	 */
	public void setNumClosestMatching(boolean value) {
		this.numClosestMatching = value;
	}

	/**
	 * Check if state covered. If not: generate covering Classifier and delete
	 * if the population is too big.
	 * 
	 * @param population
	 *            The classifier population.
	 * @param iteration
	 *            The iteration of xcsf.
	 */
	void ensureStateCoverage(Population population, int iteration) {
		if (super.size() == 0) {
			// create new classifier
			Classifier newCl = new Classifier(state, iteration);
			this.add(newCl);
			// delete from population, if necessary
			int numerositySum = 0;
			for (Classifier cl : population) {
				numerositySum += cl.getNumerosity();
			}
			int toDelete = numerositySum + 1 - XCSFConstants.maxPopSize;
			if (toDelete > 0) {
				population.deleteWorstClassifiers(toDelete);
			}
			// and finally add the new classifier
			population.add(newCl);
		}
	}

	/**
	 * Increase experience, update prediction, predictionError, setSizeEstimate
	 * and fitness of each classifier.
	 * 
	 * @see Classifier#update1(StateDescriptor)
	 * @see Classifier#update2(double, double, int)
	 */
	void updateClassifiers() {
		int size = super.size();
		double accuracySum = 0.;
		int numerositySum = 0;
		double[] accuracies = new double[size];
		// update experience, prediction and predictionError
		for (int i = 0; i < size; i++) {
			Classifier cl = super.get(i);
			cl.update1(state);
			// calculate accuracies, accuracySum & numerositySum
			accuracies[i] = cl.getAccuracy();
			accuracySum += accuracies[i] * cl.getNumerosity();
			numerositySum += cl.getNumerosity();
		}
		// update setSizeEstimate and fitness
		for (int i = 0; i < size; i++) {
			super.get(i).update2(accuracies[i], accuracySum, numerositySum);
		}
	}

	/**
	 * Adds all matching classifiers in <code>population</code> to this
	 * matchset.
	 * 
	 * @param population
	 *            The set of classifiers, that is checked for matching.
	 */
	private void addMatchingClassifiers(Population population) {
		if (!this.numClosestMatching) {
			// ---[ standard matching ]---
			for (Classifier cl : population) {
				if (cl.doesMatch(this.state)) {
					this.add(cl);
				}
			}
		} else {
			// ---[ numClostest matching ]---
			int size = population.size();
			Classifier[] clSetHelp = new Classifier[size];
			int[] nums = new int[size];
			double[] votes = new double[size];
			for (int i = 0; i < size; i++) {
				clSetHelp[i] = population.get(i);
				votes[i] = clSetHelp[i].getActivity(this.state);
				nums[i] = clSetHelp[i].getNumerosity();
			}
			int firstNum = XCSFUtils.putNumFirstObjectsFirst(votes, nums,
					clSetHelp, size, XCSFConstants.numClosestMatch);
			// now add the classifiers to this match set.
			for (int i = 0; i < firstNum; i++) {
				this.add(clSetHelp[i]);
			}
		}
	}
}
