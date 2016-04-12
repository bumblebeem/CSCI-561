////standardize还没写啊！！
//
////Substitution - the data structure to keep track of the substitution
//public class Substitution {
//    public String[] substitution;
//	
//	public Substitution() {
//		// TODO Auto-generated constructor stub
//		//because the variable is a-z, thus can use a array of 26 elements to represent them
//		substitution = new String[26];
//		for(int i = 0; i < 26; i ++){
//			substitution[i] = "";
//		}
//	}
//	
//	public boolean IsEmpty(){
//		for(int i = 0; i < 26; i ++){
//			if(substitution[i] == ""){
//				continue;
//			}
//			else{
//				return false;
//			}
//		}
//		return true;
//	}
//	
//	public void PrintSubstitution(){
//		System.out.println("This is the substitution now: ");
//		for(int i = 0; i < 26; i ++){
//			System.out.println(substitution[i]);
//		}
//		System.out.println("This is the end of the substitution.");
//	}
//	
//	public void SetFailure(){
//		for(int i = 0; i < 26; i ++){
//			substitution[i] = "FAIL";
//		}
//	}
//}
