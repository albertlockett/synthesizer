import java.io.*;
import java.util.*;

public class test{

	public static void main(String argv[]){
		FileReader file;
		BufferedReader reader;
		String line;
		String old_line = new String("...");
		Scanner sc;
		String cmd;
		int val;
		while(true){
			try{
				Thread.sleep(1000);
				file = new FileReader("proc_msg");
				reader = new BufferedReader(file);
				line = reader.readLine();
				sc = new Scanner(line);
				cmd = sc.next();
				val = sc.nextInt();
				System.out.println(line);
			} catch(Exception e){
				System.out.println("Couldn't read from file");
			}

		}
	}

}
