#! /bin/usr/perl

$start = 0;
$instrument = "none\n";

system("gcc piano_voice.c -o piano -lasound -lpthread");
system("gcc harpsicord_voice.c -o harpsicord -lasound -lpthread");

print "Welcome to the synth program\n";
print "type 'help' for list of commands\n";

while(true){
	print("SYNTH > ");
	$input = <STDIN>;

	if($input =~ m/start piano/){
		if($start == 0){
			open(CONFIG, ">config");
			print CONFIG "1";
			close(CONFIG);
			system("./piano &");
			$start = 1;
			$instrument = "piano\n";
		} else {
			print "Systen is already started\n";
		}

	}

	if($input =~ m/stop/){
		if($start == 1){
			open(CONFIG, ">config");
			print CONFIG "0";
			close(CONFIG);
			$start = 0;
			$instrument = "none\n";
		} else {
			print "system is not running\n";
		}
	}

	if($input =~ m/start harpsicord/){
		if($start == 0){
			open(CONFIG, ">config");
			print CONFIG "1";
			close(CONFIG);
			system("./harpsicord &");
			$start = 1;
			$instrument = "harpsicord\n";
		} else {
			print "Systen is already started\n";
		}

	}


	if($input =~ m/quit/){
		if($start == 1){
			open(CONFIG, ">config");
			print CONFIG "0";
			close(CONFIG);
		}
		exit;
	}
	
	if($input =~ m/w/){
		print $instrument;

	}
	
	if($input =~ m/start\n/){
		print "usage: start <piano/harpsicord>\n";
	}

	if($input =~ m/help/){
		print "COMMANDS:\n";
		print "start\t-\tstarts the instrument of your choice (valid options are 'piano' or 'harpsicord')\n";
		print "stop\t-\tstops the synthesizer from running\n";
		print "w\t-\ttells which instrument is running\n";
		print "quit\t-\texits the synthesizer\n";
	}

}
