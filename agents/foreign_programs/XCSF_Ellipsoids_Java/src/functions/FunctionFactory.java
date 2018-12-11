package functions;

import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;

/**
 * Factory class produces <code>Function</code> objects.
 * 
 * @author Patrick O. Stalph
 */
public class FunctionFactory {

	/**
	 * Factory method produces a <code>Function</code> object using the given
	 * parameters.
	 * 
	 * @param functionName
	 *            the name of the function.
	 * @param scale
	 *            the function scale.
	 * @param modifier
	 *            the function modifier.
	 * @param noiseDeviation
	 *            the (gaussian) noise deviation.
	 * @param dim
	 *            the input dimension of the function.
	 * @return the function.
	 * @throws IllegalArgumentException
	 *             if the function/dimension is not available.
	 */
	public static Function getFunction(String functionName, double scale,
			double modifier, double noiseDeviation, int dim) {
		String name = functionName.trim().toLowerCase();
		if (name.equals("sine")) {
			return new SineFunction(scale, modifier, noiseDeviation, dim);
		} else if (name.equals("sine2")) {
			return new SineFunction2(scale, modifier, noiseDeviation, dim);
		} else if (name.equals("radial")) {
			return new RadialFunction(scale, modifier, noiseDeviation, dim);
		} else if (name.equals("radialsine")) {
			return new RadialSineFunction(scale, modifier, noiseDeviation, dim);
		} else if (name.equals("crossedridge") && dim == 2) {
			return new CrossedRidgeFunction(scale, noiseDeviation);
		} else if (name.equals("sineinsine") && dim == 2) {
			return new SineInSineFunction(scale, modifier, noiseDeviation);
		} else {
			throw new IllegalArgumentException(
					"Function and/or dimension not available: '" + name + "' "
							+ dim + "D.");
		}
	}

	/**
	 * Factory method produces a <code>Function</code> array using the given
	 * parameters.
	 * 
	 * @param functionNames
	 *            the comma-seperated names of the functions.
	 * @param scale
	 *            the function scale.
	 * @param modifier
	 *            the function modifier.
	 * @param noiseDeviation
	 *            the (gaussian) noise deviation.
	 * @param dim
	 *            the input dimension of the function.
	 * @return the function.
	 * @throws IllegalArgumentException
	 *             if one function/dimension is not available.
	 */
	public static Function[] getFunctions(String functionNames, double scale,
			double modifier, double noiseDeviation, int dim) {
		String[] splited = functionNames.split(",");
		int n = splited.length;
		Function[] functions = new Function[n];
		for (int i = 0; i < n; i++) {
			functions[i] = getFunction(splited[i].trim().toLowerCase(), scale,
					modifier, noiseDeviation, dim);
		}
		return functions;
	}

	/**
	 * Loads the properties file from <code>filename</code>, parses
	 * functionNames, -dimension, -scale, -modifier and -noiseDeviation and
	 * returns the corresponding <code>Function</code> array using
	 * {@link #getFunctions(String, double, double, double, int)}.
	 * 
	 * @param filename
	 *            the properties file.
	 * @return the functions specified in <code>filename</code>.
	 * @throws IOException
	 *             if the file is not found or if the file is not properly
	 *             formatted.
	 * @throws IllegalArgumentException
	 *             if one function/dimension is not available.
	 */
	public static Function[] loadFromPropertiesFile(String filename)
			throws IOException, IllegalArgumentException {
		FileInputStream in = new FileInputStream(filename);
		Properties p = new Properties();
		p.load(in);
		String functionNames = p.getProperty("functions");
		int dim = Integer.parseInt(p.getProperty("functionInputDimension"));
		double scale = Double.parseDouble(p.getProperty("functionScale"));
		double modifier = Double.parseDouble(p.getProperty("functionModifier"));
		double noiseDeviation = Double.parseDouble(p
				.getProperty("functionNormNoiseDeviation"));
		in.close();
		return getFunctions(functionNames, scale, modifier, noiseDeviation, dim);
	}
}
