import java.util.*;
import java.io.*;

public class inference {
	public static int num_of_queries;
	public static int num_of_clauses;
	public static ArrayList<String> queries = new ArrayList<String>();
	public static ArrayList<String> clauses = new ArrayList<String>();	//this is the KB
	public static ArrayList<String> facts = new ArrayList<String>();
	public static ArrayList<String> implications = new ArrayList<String>();
	public static int standardize_help = 0;
	public static boolean isCompond = false;
	
	//@#$% add an static argument for loop detection?
	public static ArrayList<String> loop_detection = new ArrayList<String>();
	
	//main function
	public static void main(String args[]) throws FileNotFoundException{
		//Scanner input_file = new Scanner(new File("/Users/mfx/Documents/input.txt"));
		BufferedReader reader = null;
		BufferedWriter writer = null;
		File input_file = null;
		try {
			writer = new BufferedWriter(new FileWriter("/Users/mfx/Documents/output.txt"));
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		
		if(args.length > 0){
			System.out.println(args[0]);
//			System.out.println(args[1]);
			input_file = new File(args[0]);
		}
		reader = new BufferedReader(new FileReader(input_file));
			
		//deal with input file, i.e. initialize num_of_queries, queries, num_of_clauses, clauses
		try {
			num_of_queries = Integer.parseInt(reader.readLine().trim());
			int temp = num_of_queries;
			while(temp -- > 0){
				queries.add(reader.readLine().trim());
			}
			num_of_clauses = Integer.parseInt(reader.readLine().trim());
			temp = num_of_clauses;
			while(temp -- > 0){
				clauses.add(reader.readLine().trim());
			}
		} catch (NumberFormatException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
			
		//divide clauses(KB) into facts and implications
		for(int i = 0; i < clauses.size(); i ++){
			if(clauses.get(i).contains("=>")){
				implications.add(clauses.get(i));
			}
			else{
				facts.add(clauses.get(i));
			}
		}
			
		//deal with each query with backward chaining algorithm
		for(int i = 0; i < queries.size(); i ++){
			String query = queries.get(i);
			//remember to clear loop_detection and reset standardize_help every time!
			//because they are global
			loop_detection.clear();
			standardize_help = 0;
			//firstly check if the query is already in the KB
			if(alreadyInKB(query)){
				System.out.println("TRUE");
				try {
					writer.write("TRUE\n");
					writer.flush();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
			else{
				//if not, do backward chaining to find out
				if(BC_Ask(query)){
					System.out.println("TRUE");
					try {
						writer.write("TRUE\n");
						writer.flush();
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
				else{
					System.out.println("FALSE");
					try {
						writer.write("FALSE\n");
						writer.flush();
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
			System.out.println("****************");
			System.out.println("****************");
			System.out.println("This is the end of case " + (i+1));
			System.out.println("****************");
			System.out.println("****************");
		}
			
		try {
			if(reader != null)
				reader.close();
			if(writer != null){
				writer.close();
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	//Backward Chaining Ask
	public static boolean BC_Ask(String query){
		HashMap<String, String> substitution = new HashMap<String, String>();
		ArrayList<HashMap<String, String>> result = BC_Or(query, substitution);
		
		if(result.size() == 0){
			return false;
		}
		else{
			System.out.println("@#$% THE SUCCESS TRANSFORMATION IS " +  result);
			return true;
		}
	}
	
	//test whether the query is already in the KB
	public static boolean alreadyInKB(String query){
		for(int i = 0; i < facts.size(); i ++){
			if(facts.get(i).trim().equals(query.trim())){
				return true;
			}
		}
		return false;
	}
	
	//get consequence from a implication
	public static String getConsequenceFromImplication(String implication){
		String[] temp = implication.split("=>");
		return temp[1].trim();
	}
	
	//get premises from a implication
	public static ArrayList<String> getPremisesFromImplication(String implication){
		String[] temp = implication.split("=>");
		//这里必须要用\\，也是醉了
		String[] sub_premises = temp[0].trim().split("\\^");
		ArrayList<String> premises = new ArrayList<String>();
		for(int i = 0; i < sub_premises.length; i ++){
			premises.add(sub_premises[i].trim());
		}
		return premises;
	}
	
	//helper of standardize_variables
	public static String Standardize_Variables_Helper(String original_line, String additional_index){
		String result = new String();
		ArrayList<String> variables = getVariablesFromSentence(original_line);
		int num_of_variables = NumOfArguments(original_line);
		for(int i = 0; i < variables.size(); i ++){
			String temp = variables.get(i) + additional_index;
			variables.set(i, temp);
			System.out.println("@#$%"+ variables.get(i));
		}
		
		int index = original_line.indexOf("(");
		result += original_line.substring(0, index + 1);
		for(int i = 0; i < num_of_variables - 1; i++){
			result += (variables.get(i) + ",");
		}
		result += variables.get(num_of_variables - 1);
		result += ")";
		
		return result;
	}
	
	//standardize the variables, in other words, change the variables in the original_line into
	//variables that never turn up before
	public static String Standardize_Variables(String original_line) {
		String result = new String();
		String additional_index = Integer.toString(standardize_help);
			
		if(original_line.contains("=>")){
			String consequence = getConsequenceFromImplication(original_line);
			ArrayList<String> premises = getPremisesFromImplication(original_line);
			String temp_result_2 = Standardize_Variables_Helper(consequence, additional_index);
			//deal with the premises
			String temp_result_1 = new String();
			//要把premises分成一个一个的，处理之前一串premises的size还是1。。。
			for(int i = 0; i < premises.size() - 1; i ++){
				temp_result_1 += Standardize_Variables_Helper(premises.get(i), additional_index);
				temp_result_1 += " ^ ";
			}
			temp_result_1 += Standardize_Variables_Helper(premises.get(premises.size() - 1), additional_index);
			result = temp_result_1 + " => " + temp_result_2;
		}
		else{
			result = Standardize_Variables_Helper(original_line, additional_index);
		}
		
		standardize_help ++;
		return result;
	}
	
	//judge if the sentence is a fact
	public static boolean IsFact(String original_line) {
		if(facts.contains(original_line)){
			return true;
		}
		return false;
	}

	//check if all the arguments in the goal are constants
	public static boolean AllConstant(String goal) {
		ArrayList<String> variables = getVariablesFromSentence(goal);
		for(int i = 0; i < variables.size(); i ++){
			ArrayList<String> temp = new ArrayList<String>(variables.subList(i, i + 1));
			if(! IsConstant(temp)){
				return false;
			}
		}
		return true;
	}
	
	//FOL-BC-OR
	public static ArrayList<HashMap<String, String>> BC_Or(String goal, HashMap<String, String> theta){	
		System.out.println("#########We are going inside the BC_Or!!!!!!!!");
		System.out.println("This time the goal is " + goal);
		//matched_consequences refer to the consequences have the same predicate with a certain query
		ArrayList<String> matched_consequences = new ArrayList<String>();
		ArrayList<HashMap<String, String>> or_unifications = new ArrayList<>();
		
		//@#$% loop detection
		if(loop_detection.contains(goal)){
			return or_unifications;
		}
		
		if((!goal.contains("=>")) && AllConstant(goal)){
			loop_detection.add(goal);
		}
		
		//"for each rule(lhs => rhs) in FETCH-RULES-FOR-GOAL(KB,goal)"
		for(int i = 0; i < implications.size(); i ++){
			String consequence = getConsequenceFromImplication(implications.get(i));
			if(IsSamePredicate(goal, consequence)){
				matched_consequences.add(implications.get(i));
			}
		}
		for(int i = 0; i < facts.size(); i ++){
			if(IsSamePredicate(goal, facts.get(i))){
				matched_consequences.add(facts.get(i));
			}
		}
		
		//@#$% this is crucial
		if(matched_consequences.size() == 0){
			return or_unifications;
		}
		
		//(lhs, rhs) <- STANDARDIZE-VARIABLES((lhs, rhs))
		for(int i = 0; i < matched_consequences.size(); i ++){		
			System.out.println("NOW WE ARE EXAMINING: " + matched_consequences.get(i));
			String original_line = matched_consequences.get(i);
			String line = new String();
			
			//@#$% 如果都是常量这里是不是也应该standardize一下。。？还是这里把没用的substitution删了？
			//fisrtly we standardize the variables!
			if(IsFact(original_line)){
				line = original_line;
			}
			else{
				line = Standardize_Variables(original_line);
				System.out.println("The standardize result is " + line);
			}
			
			if(line.contains("=>")){
				isCompond = false;
				//lhs is premises, rhs is consequence
				//"for each theta' in FOL-BC-AND(KB, lhs, UNIFY(rhs,goal,theta))"
				String consequence = getConsequenceFromImplication(line);
				ArrayList<String> premises = getPremisesFromImplication(line);
				
				//get the substring of the variables in the query and consequence
				ArrayList<String> variables_of_consequence = getVariablesFromSentence(consequence);
				ArrayList<String> variables_of_goal = getVariablesFromSentence(goal);
				HashMap<String, String> unifies = Unify(variables_of_consequence, variables_of_goal, theta);
					
				ArrayList<HashMap<String, String>> unifications = BC_And(premises, unifies);

				//@#$% if是新加的
				if(! unifications.isEmpty()){
					for(HashMap<String, String> s : unifications){
						if(! or_unifications.contains(s)){
							or_unifications.add(s);
						}
					}
				}
			}
			else{
				isCompond = true;
				String consequence = line;
				ArrayList<String> variables_of_consequence = getVariablesFromSentence(consequence);
				ArrayList<String> variables_of_goal = getVariablesFromSentence(goal);
				HashMap<String, String> unifies = Unify(variables_of_consequence, variables_of_goal, theta);
					
				ArrayList<HashMap<String, String>> unifications = new ArrayList<HashMap<String, String>>();
				if(unifies != null){
					unifications.add(unifies);
				}
				
				//@#$% if是新加的
				if(! unifications.isEmpty()){
					for(HashMap<String, String> s : unifications){
						if(! or_unifications.contains(s)){
							or_unifications.add(s);
						}
					}
				}
			}
			//最后上边这里加一个函数判断matched_consequences里的element是不是已经是KB中的fact了

		}
		
		return or_unifications;
	}

	//FOL-BC-AND
	public static ArrayList<HashMap<String, String>> BC_And(ArrayList<String> goals, HashMap<String, String> theta){
		System.out.println("#########We are going inside the BC_And!!!!!!!!");
		ArrayList<HashMap<String, String>> unifications = new ArrayList<>();
		
		//@#$% 这个判断有点问题！！用isEmpty() or size() == 0 or == null
		if(theta == null){
			return unifications;
//			return null;
		}
		else if(goals == null || goals.size() == 0){
			if(! unifications.contains(theta)){
				unifications.add(theta);
			}
			return unifications;
		}
		else{
			//first, rest <- FIRST(goals), REST(goals)
			String first = goals.get(0);
			ArrayList<String> rest = new ArrayList<String>();
			for(int i = 0; i < goals.size(); i ++){
				rest.add(goals.get(i));
			}
			rest.remove(0);			
			//上边几行rest的处理很重要啊，rest有可能为0的啊！！所以还是需要把goals中的元素都加到rest中，之后再remove掉第一个元素
			
			ArrayList<HashMap<String, String>> or_unifications = BC_Or(Subst(theta, first), theta);
			
			for(HashMap<String, String> or_unifier : or_unifications){
				ArrayList<HashMap<String, String>> and_unifications = BC_And(rest, or_unifier);
				for(HashMap<String, String> and_unifier : and_unifications){
					if(! unifications.contains(and_unifier)){
						unifications.add(and_unifier);
					}
				}
			}
		}
		
		return unifications;
	}

	//apply the substitution from theta to the sentence
	public static String Subst(HashMap<String, String> theta, String first) {
		String result = new String();
		int num_of_variables = NumOfArguments(first);
		ArrayList<String> variables_of_first = getVariablesFromSentence(first);
		if(theta.isEmpty()){
			return first;
		}
		else{
			for(int i = 0; i < variables_of_first.size(); i ++){
				//@#$% containsKey or containsValue?? should be constainsKey!
				if(theta.containsKey(variables_of_first.get(i))){
					variables_of_first.set(i, theta.get(variables_of_first.get(i)));
				}
			}
		}
		
		int temp = first.indexOf("(");
		//Attention! In the substring(beginIndex, endIndex), 
		//the beginIndex is inclusive while the endIndex is exclusive...!!!
		result += first.substring(0, temp + 1);
		for(int i = 0; i < num_of_variables - 1; i++){
			result += (variables_of_first.get(i) + ",");
		}
		result += variables_of_first.get(num_of_variables - 1);
		result += ")";
		
		System.out.println("So, the substitution result is :" + result);
		return result;
	}

	//get variables from a certain sentence
	public static ArrayList<String> getVariablesFromSentence(String sentence){
		int temp = sentence.indexOf("(");
		String sub_of_sentence = sentence.substring(temp + 1, sentence.length() - 1).trim();
		//divide the substring of arguments into strings contain only one argument, and return
		String[] temp1 = sub_of_sentence.split(",");
		ArrayList<String> result = new ArrayList<String>();
		for(int i = 0; i < temp1.length; i ++){
			result.add(temp1[i]);
		}
		return result;
	}

	//figure out if l is a constant
	public static boolean IsConstant(ArrayList<String> l){
		if(l.size() != 1){
			return false;
		}
		else{
			if(l.get(0).charAt(0) >= 'a' && l.get(0).charAt(0) <= 'z'){
				return false;
			}
		}
		return true;
	}
	
	//figure out if l is a variable
	public static boolean IsVariable(ArrayList<String> l){
		if(l.size() == 1){
			if(l.get(0).charAt(0) >= 'a' && l.get(0).charAt(0) <= 'z'){
				return true;
			}
		}
		return false;
	}
	
	//unify function according to the book
	public static HashMap<String, String> Unify(ArrayList<String> consequence, ArrayList<String> goal, HashMap<String, String> theta){
		if(theta == null){
			return null;
		}
		else if(IsConstant(consequence) && IsConstant(goal) && consequence.get(0).equals(goal.get(0))){
			return theta;
		}
		else if(IsVariable(consequence)){
			return Unify_Var(consequence, goal, theta);
		}
		else if(IsVariable(goal)){
			return Unify_Var(goal, consequence, theta);
		}
		else if(consequence.size() > 1 && goal.size() > 1){
			ArrayList<String> temp1 = new ArrayList<String>(consequence.subList(1, consequence.size()));
			ArrayList<String> temp2 = new ArrayList<String>(goal.subList(1, goal.size()));
			ArrayList<String> temp3 = new ArrayList<String>(consequence.subList(0, 1));
			ArrayList<String> temp4 = new ArrayList<String>(goal.subList(0, 1));
			return Unify(temp1, temp2, Unify(temp3, temp4, theta));
		}
		else{
			return null;
		}
	}
	
	//unify_var function according to the book
	//@#$% 错误在这个函数里
	public static HashMap<String, String> Unify_Var(ArrayList<String> consequence, ArrayList<String> x, HashMap<String, String> theta){
		//@#$% 这里的keySet()是不是也可以分别用containsKey(), containsValue()?
		if(theta.keySet().contains(consequence.get(0))){
			ArrayList<String> temp1 = new ArrayList<String>();
			temp1.add(theta.get(consequence.get(0)));
			return Unify(temp1, x, theta);
		}
		else if(theta.keySet().contains(x.get(0))){
			ArrayList<String> temp2 = new ArrayList<String>();
			temp2.add(theta.get(x.get(0)));
			return Unify(consequence, temp2, theta);
		}
		else if(Occur_Check(theta, consequence, x)){
			return null;
		}
		//@#$% 现在的错误可能就出现在这个else里边
		else{
			//@#$% 这里consequence和x反没反啊。。？应该是没有反
			theta.put(consequence.get(0), x.get(0));
			return theta;
		}
	}

	//do occur check
	public static boolean Occur_Check(HashMap<String, String> theta, ArrayList<String> consequence,
			ArrayList<String> x) {
		//@#$% could be finished, but not necessary in this assignment
		return false;
	}
	
	//return the number of arguments(variables) in the sentence
	public static int NumOfArguments(String sentence){
		String[] temp = sentence.split(",");
		return temp.length;
	}
	
	//figure out if the predicates of two sentences are the same
	public static boolean IsSamePredicate(String query, String consequence){
		//to figure out whether the substring before (..,..,..,..) is same or not
		
		//the following "NumOfArguments(query) == NumOfArguments(consequence)"
		//seems unnecessary, because I didn't notice this following sentence
		//in the description of the homework: 
		//"Same predicate will not appear with different number of arguments"
		//@#$% BUT THE SIGNATURE COULD BE DIFFERENT!!!!!! SO HERE IS A MISTAKE!!!!!!
		if(NumOfArguments(query) == NumOfArguments(consequence)
				&& query.substring(0, query.indexOf("(")).equals(consequence.substring(0, consequence.indexOf("(")))){
			return true;
		}
		return false;
	}
	
}