
function genIdentController(authCode)
	return {COMMAND="IDENTIFY", CONTENT="CONTROLLER", AUTHCODE=tostring(authCode)}
end

function genIdentBot()
	return {COMMAND="IDENTIFY", CONTENT="BOT"}
end
