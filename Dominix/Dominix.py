import twisted #@UnusedImport
from twisted.internet.protocol import Factory
from twisted.protocols.basic import LineReceiver
from twisted.internet import reactor
from twisted.internet.reactor import run #@UnresolvedImport @UnusedImport
import json

class Dominix(LineReceiver):

	def __init__(self, connections, controllers, bots, authCode):
		self.connections = connections
		self.controllers = controllers
		self.bots = bots
		self.name = None
		self.authCode = authCode
		self.state = "IDENTIFICATION"
		self.responseTarget = None

	def connectionMade(self):
		if(self not in self.connections):
			self.connections.append(self)
		self.sendLine(json.dumps({"COMMAND":"IDENTIFY"}))

	def connectionLost(self, reason):
		if(self in self.connections):
			self.connections.remove(self)
		if(self in self.controllers):
			self.controllers.remove(self)
		if(self in self.bots):
			self.bots.remove(self)

	def lineReceived(self, line):
		if self.state == "BOT":
			self.handle_BOT(line)
		elif(self.state=="CONTROLLER"):
			self.handle_CONTROLLER(line)
		elif self.state == "IDENTIFICATION":
			self.handle_IDENTIFICATION(line)
		else:
			self.handle_FAIL(line)

	def handle_CONTROLLER(self, line):
		try:
			msg = json.loads(line)
		except:
			return
		if(msg["COMMAND"]=="RELAY"):
			content = msg["CONTENT"]
			for protocol in self.bots:
				protocol.responseTarget = self
				protocol.sendLine(json.dumps(content))
	
	def handle_BOT(self, line):
		try:
			msg = json.loads(line)
		except:
			return
		if(msg["COMMAND"] == "RESPOND" and self.responseTarget):
			self.responseTarget.sendLine(json.dumps(msg["CONTENT"]))

	def handle_IDENTIFICATION(self, line):
		try:
			msg = json.loads(line)
		except:
			return
		if(msg["COMMAND"]=="IDENTIFY"):
			if(msg["CONTENT"]=="BOT"):
				self.state = "BOT"
				self.bots.append(self)
				self.sendLine(json.dumps({"COMMAND":"STATUS", "CONTENT":{"IDENT":"SUCCESS", "STATE":"BOT"}}))
			elif(msg["CONTENT"]=="CONTROLLER"):
				if(msg["AUTHCODE"] == self.authCode):
					self.state = "CONTROLLER"
					self.controllers.append(self)
					self.sendLine(json.dumps({"COMMAND":"STATUS", "CONTENT":{"IDENT":"SUCCESS", "STATE":"CONTROLLER"}}))
				else:
					self.state = "IDENTIFICATION"
					self.sendLine(json.dumps({"COMMAND":"STATUS", "CONTENT":{"IDENT":"FAILURE"}}))


class DominixFactory(Factory):

	def __init__(self, authCode):
		self.connections = [] # maps user names to Chat instances
		self.controllers = []
		self.bots = []
		self.authCode = authCode

	def buildProtocol(self, addr):
		return Dominix(self.connections, self.controllers, self.bots, self.authCode)


reactor.listenTCP(8123, DominixFactory("DSX")) #@UndefinedVariable
reactor.run() #@UndefinedVariable
