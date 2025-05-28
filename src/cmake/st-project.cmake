function(add_st_target_properties TARGET_NAME)
	target_link_options(${TARGET_NAME} PRIVATE -Wl,-Map=$<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}.map)
	target_link_options(${TARGET_NAME} PRIVATE -Wl,--print-memory-usage)

	add_custom_command(
		TARGET ${TARGET_NAME} POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} -O ihex
		$<TARGET_FILE:${TARGET_NAME}> ${TARGET_NAME}.hex
	)

	add_custom_command(
		TARGET ${TARGET_NAME} POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} -O binary
		$<TARGET_FILE:${TARGET_NAME}> ${TARGET_NAME}.bin
	)

	add_custom_command(
		TARGET ${TARGET_NAME} POST_BUILD
		COMMAND ${CMAKE_OBJDUMP} -h -S $<TARGET_FILE:${TARGET_NAME}> > ${TARGET_NAME}.list
	)
	if(STM32_SIGNINGTOOL_CLI)
		add_custom_command(
			TARGET ${TARGET_NAME} POST_BUILD
			COMMAND ${STM32_SIGNINGTOOL_CLI} --silent --binary-image ${TARGET_NAME}.bin --no-keys --option-flags 0x80000000 --type fsbl --output ${TARGET_NAME}-trusted.bin --header-version 2.3 --dump-header ${TARGET_NAME}-trusted.bin
		)
	endif()
endfunction()
