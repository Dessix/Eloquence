import twisted #@UnusedImport
from twisted.internet.protocol import Factory
from twisted.protocols.basic import LineReceiver
from twisted.internet import reactor
from twisted.internet.reactor import run #@UnresolvedImport @UnusedImport
import uuid
import json

class Dominix(LineReceiver):

	def __init__(self, connections, authCode):
		self.connections = connections
		self.name = None
		self.authCode = authCode
		self.state = "IDENTIFICATION"
		self.responseTarget = None

	def connectionMade(self):
		if(self not in self.connections):
			self.connections.append(self)
			self.id = str(uuid.uuid4())
			print("join "+str(self.id))
		self.sendLine(json.dumps({"COMMAND":"IDENTIFY", "ID":self.id}))

	def connectionLost(self, reason):
		if(self in self.connections):
			self.connections.remove(self)
			print("part "+str(self.id))

	def lineReceived(self, line):
		try:
			msg = json.loads(line)
		except:
			print("Protocol Failure")
			self.transport.loseConnection()
			return
		print(json.dumps(msg))
		if self.state == "BOT":
			self.handle_BOT(msg)
		elif(self.state=="CONTROLLER"):
			self.handle_CONTROLLER(msg)
		elif self.state == "IDENTIFICATION":
			self.handle_IDENTIFICATION(msg)
		else:
			print("Protocol Failure")
			self.transport.loseConnection()
			return
	
	def handle_CONTROLLER(self, msg):
		print("Handling controller request...")
		if(msg["COMMAND"]=="RELAY"):
			bots = [i for i in self.connections if i.state=="BOT"]
			content = json.dumps({"COMMAND":"RELAYED", "SENDER":self.id, "CONTENT":msg["CONTENT"]})
			if(msg.has_key("TARGET")):
				target = msg["TARGET"]
				bots = [i for i in bots if i.id.find(target) >= 0]
			for protocol in bots:
				protocol.responseTarget = self
				protocol.sendLine(content)
	
	def handle_BOT(self, msg):
		print("Handling bot request...")
		if(msg["COMMAND"] == "RESPOND" and msg.has_key("TARGET")):
			if(msg["TARGET"] == "LAST"):
				self.responseTarget.sendLine(json.dumps({"COMMAND":"RESPONSE", "SENDER":self.id, "CONTENT":msg["CONTENT"]}))
			else:
				for target in [conn for conn in self.connections if conn.state == "CONTROLLER" and conn.id.find(msg["TARGET"]) >= 0]:
					target.sendLine(json.dumps({"COMMAND":"RESPONSE", "SENDER":self.id, "CONTENT":msg["CONTENT"]}))

	def handle_IDENTIFICATION(self, msg):
		print("Handling identification request...")
		if(msg["COMMAND"]=="IDENTIFY"):
			if(msg["CONTENT"]=="BOT"):
				self.state = "BOT"
				message = json.dumps({"COMMAND":"STATUS", "CONTENT":{"IDENT":"SUCCESS", "STATE":"BOT"}})
				print(message)
				self.sendLine(message)
			elif(msg["CONTENT"]=="CONTROLLER"):
				if(msg["AUTHCODE"] == self.authCode):
					self.state = "CONTROLLER"
					message = json.dumps({"COMMAND":"STATUS", "CONTENT":{"IDENT":"SUCCESS", "STATE":"CONTROLLER"}})
					print(message)
					self.sendLine(message)
				else:
					self.state = "IDENTIFICATION"
					message = json.dumps({"COMMAND":"STATUS", "CONTENT":{"IDENT":"FAILURE"}})
					print(message)
					self.sendLine(message)


class DominixFactory(Factory):

	def __init__(self, authCode):
		self.connections = [] # maps user names to Chat instances
		self.authCode = authCode

	def buildProtocol(self, addr):
		return Dominix(self.connections, self.authCode)


reactor.listenTCP(8123, DominixFactory("DSX")) #@UndefinedVariable
reactor.run() #@UndefinedVariable
