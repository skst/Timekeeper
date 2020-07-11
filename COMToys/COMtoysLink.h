#if defined(_WIN64)
	#if defined(_AFXDLL)
		#error __FILE__ ": We don't support MFC Shared library"
	#else
		#if defined(_UNICODE)
			#if defined(_DEBUG)
				#pragma message("Using COMtoys library Unicode 64-bit Debug...")
				#pragma comment(lib, "ctoysx64UD.lib")
			#else
				#pragma message("Using COMtoys library Unicode 64-bit Release...")
				#pragma comment(lib, "ctoysx64U.lib")
			#endif
		#else
			#if defined(_DEBUG)
				#pragma message("Using COMtoys library 64-bit Debug...")
				#pragma comment(lib, "ctoysx64D.lib")
			#else
				#pragma message("Using COMtoys library 64-bit Release...")
				#pragma comment(lib, "ctoysx64.lib")
			#endif
		#endif
	#endif
#else
	#if defined(_AFXDLL)
		#error __FILE__ ": We don't support MFC Shared library"
	#else
		#if defined(_UNICODE)
			#if defined(_DEBUG)
				#pragma message("Using COMtoys library Unicode 32-bit Debug...")
				#pragma comment(lib, "ctoysUD.lib")
			#else
				#pragma message("Using COMtoys library Unicode 32-bit Release...")
				#pragma comment(lib, "ctoysU.lib")
			#endif
		#else
			#if defined(_DEBUG)
				#pragma message("Using COMtoys library 32-bit Debug...")
				#pragma comment(lib, "ctoysD.lib")
			#else
				#pragma message("Using COMtoys library 32-bit Release...")
				#pragma comment(lib, "ctoys.lib")
			#endif
		#endif
	#endif
#endif
