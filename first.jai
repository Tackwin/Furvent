#import "Compiler";

#run build();

build :: () {
	set_build_options_dc(.{do_output=false});

	options := get_build_options();
	options.output_executable_name = "Fortune";
	options.output_type = .EXECUTABLE;

	w := compiler_create_workspace("Main");
	set_build_options(options, w);

	compiler_begin_intercept(w);
	add_build_file("src/main.jai", w);

	while true {
		message := compiler_wait_for_message();
		if !message break;

		if message.kind == {
			case .COMPLETE;
				break;
		}
	}

	compiler_end_intercept(w);
}