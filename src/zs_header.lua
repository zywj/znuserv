
function get_header(c, ld)

	local header_str = "HTTP/1.1 "

	header_str = header_str .. c

	if c == 200 then 
		header_str = header_str .. " OK\r\n"
	elseif c == 304 then
		header_str = header_str .. " Not Modified\r\n"
	elseif c == 400 then
		header_str = header_str .. " Bad Request\r\n"
	elseif c == 404 then 
		header_str = header_str .. " Not Found\r\n"
	else
		header_str = header_str
	end

	header_str = header_str .. "Server: znuserv\r\n"
	header_str = header_str .. "Content_Type: text/html\r\n"
	header_str = header_str .. "Connection: close\r\n"	

	header_str = header_str .. "Last-Modified: " .. ld .. "\r\n\r\n"

	return header_str
end