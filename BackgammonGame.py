
from CameraController import CameraController
#from AutoPlayer import AutoPlayer
from gnubg import GNUBG
from DiceController import DiceController
from ScreenController import ScreenController
from DataModel import  DataModel
from ArduinoController import ArduinoController
from BoardController import BoardController
from BoardSimulation import BoardSimulation
import json
import os
import random
import time
import cv2
import time
from DicePrediction import predict_label
from DicePrediction import load_model


from AuxFunctions import AuxFunctions
af = AuxFunctions ()



class BackgammonGame:
    USER = 1

    arduino = 0
    data_model = 0
    screen = 0
    camera = 0
    board = 0
    dice = 0
    auto_player = 0
    gnubg = 0
    board_simulation = 0
    STATE_CALIBRATION,STATE_DETECTION,STATE_PLAYER,STATE_AUTO_PLAYER,STATE_CHOOSE_PLAYER=0,1,2,3,4

    
    cur_state = STATE_DETECTION

    def LoadBoardParams(self):
        if os.path.exists("BoardController.json"):
            with open("BoardController.json", "r") as file:
                loaded_data = json.load(file)
            self.board = BoardController.FromDictionary(self.data_model,self.arduino,loaded_data)
        else:
            self.board = BoardController(self.data_model,self.arduino)

    def LoadCameraParams(self):
        if os.path.exists("CameraController.json"):
            with open("CameraController.json", "r") as file:
                loaded_data = json.load(file)
            self.camera = CameraController.FromDictionary(self.screen,self.data_model,loaded_data,self.board)
            self.camera.data_model = self.data_model
            self.camera.screen_controller = self.screen
            self.camera.board = self.board
        else:
            self.camera = CameraController(self.screen,self.data_model,self.board)
        self.board.camera = self.camera

    def InitCamera(self):
        self.LoadCameraParams()

    def InitBoard(self):
        self.LoadBoardParams()
    def __init__(self):
        
        self.data_model = DataModel()
        self.screen = ScreenController(self.data_model)
        self.arduino = ArduinoController(self.screen,self.data_model)
        #self.board = BoardController(self.data_model,self.arduino) 
        self.InitBoard()
        #self.auto_player = AutoPlayer(self.data_model)
        self.gnubg = GNUBG(self.data_model)
        self.InitCamera()
        self.arduino.camera = self.camera
        self.board_simulation = BoardSimulation(self.camera,self.data_model,self.board)
        self.camera.board_simulation = self.board_simulation
        self.dice = DiceController(self.camera,self.arduino,self.data_model,self.screen,self.data_model.use_dice)

    def InitGame(self):
        pass
            

    """def SwitchPlayer(self):
        if self.current_turn == self.AUTO_PLAYER:
            self.current_turn = self.USER
        else:
            self.current_turn = self.AUTO_PLAYER"""

    def RollDiceUntilValid(self,text = "Rolling!"):
        dice = (0,0)
        while dice == (0,0):
            dice = self.RollDice(text)
            if dice == (0,0):
                self.board.SendCommandToRGB("STARS","Rolling","Again",0,0,0,0)
        return dice

    def RollDice (self, text = "Rolling!"):
        if (text != ''):
            self.board.SendCommandToRGB("SCROLL_TEXT",text,"NONE",0,0,0,0)
        dice = (0,0)
        #dice = self.dice.RollDice()

        self.dice.SendRollDiceCommand()

        refresh_count = 0
        start_time = time.time()
        stop_wait = False
        if self.data_model.use_dice == True:
            #while time.time() - start_time < 4 and refresh_count < 40 and (not stop_wait):
            while refresh_count < 15 and (not stop_wait) or time.time() - start_time < 4:
                #print ("waiting while rolling")
                self.camera.GetBoardPlayers()
                self.camera.screen_controller.PrepareWindows()
                self.camera.screen_controller.ShowWindows()
                _,stop_wait = self.arduino.CheckConfirmtionTCPNoWait()
                refresh_count +=1
        
        dice = self.dice.RecognizeDice()


        return dice
       
    def TestRGB(self):
        num1 = random.randint(1, 6)
        num2 = random.randint(1, 6)
        self.board.SendCommandToRGB("Stars","Test" + str(num1),"Test" + str(num2),num1,num2,num1,num2)

    def ChooseTextForPlayer(self):
        player_text = ["Your move, backgammon master!",
                       "Show off your skills!",
                       "Time to shine!",
                       "Roll and rock!",
                       "Your command awaits!",
                       "Answer the call!",
                       "Yalla!",
                       "Waiting...",
                       "Your move - no pressure!",
                       "Take your time, we're not going anywhere!",
                       "Think you can beat the machine's?",
                       "The stage is set - it's all on you now!",
                       "Your turn to shine - make it count!",
                       "Ready to roll the dice and make your mark?",
                       "No rush- Play your move!",
                       "The board is waiting for your move!",
                       "The game's in your hands-what's your play?",
                       "Your move - the spotlight's on you!",
                       "Time to show who's the boss - your move",
                       "The board's waiting - what's your plan?",
                       "Tick-tock, tick-tock- Play your move!",
                       "The suspense is killing us - make your move!",
                       "All eyes on you-what's your next move?",
                       ]

        num = random.randint(0, len(player_text) - 1)
        return player_text[num]

    def ChooseTextForAutoPlayer(self):
        """auto_player_text = [
            "hi",
            "bye",
            "nice",
            "rice"]"""
        auto_player_text = [
            "Sit back and let the machine do its thing!",
            "Time for the computer to flex its digital muscles!",
            "Watch closely - the machine's making its move!",
            "Hold tight, it's the computer's turn to strategize!",
            "The computer's up next - can you match its cunning?",
            "Get ready to face the digital mastermind!",
            "Computer's move - can you anticipate its next play?",
            "Prepare to be impressed by the machine's next move!",
            "Time for the machine to make its mark!",
            "Hold onto your seats - the computer's up!",
            "Computer's turn - can you crack its code?",
            "Let's see what the AI has up its sleeve!",
            "Sit tight, the computer's calculating its move!",
            "Watch out, the digital brain is in action!",
            "It's the computer's turn - your move next!",
            "Buckle up, the computer's about to make waves!",
            "Time for the computer to strut its stuff!",
        ]
        num = random.randint(0, len(auto_player_text) - 1)
        return auto_player_text[num]
        

    def ChooseMode(self):
        modes = [
                    #"SCROLL_TEXT",
                    "BOUNCING",
                    "RAND_DOTS",
                    "SNAKE",
                    "STARS",
                    "CHECKERS"
                ]
        num = random.randint(0, len(modes) - 1)
        return modes[num]
        
    def WaitUntilImageIsValid(self):
        if self.data_model.allow_no_camera == True:
            return
        already_shown = False
        while  self.CheckValidImage() == False:
            if already_shown == False:
                self.board.SendCommandToRGB("JUST_TEXT","No Camera..","Waiting..",0,0,0,0)
                already_shown = True
            time.sleep(0.1)

    def SimulateAutoPlayer(self,color,skip_bar):
        switch = False
        if color == 'b':
            switch = True
        ret_val = False

        self.WaitUntilImageIsValid()

        self.data_model.SaveState()
        self.screen.SaveImagesToDisk()
        #self.data_model.dice = self.RollDiceUntilValid("I'm Rolling")


        #self.data_model.dice = [1,4]


        #print(self.data_model.dice)
        
        #self.board.SendCommandToRGB(self.ChooseMode(),"My Turn!",self.ChooseTextForAutoPlayer(),0,0,self.data_model.dice[0],self.data_model.dice[1])
        #movements = self.auto_player.CalcNextMove(self.data_model.dice,color)
        
        movements = self.gnubg.CalcNextMove(self.data_model.dice,switch)
        #movements2 = self.gnubg.CalcNextMove(self.data_model.dice,True)
        #print (movements2)
        #self.data_model.dice = (1,4)   #XXXXXXX
        #movements = [[15, 19], [17, 20]]  



        #self.data_model.dice = (3, 4)
        #movements = [[19, 22], [21, 26]]



        if movements == 'resign':
            return 'resign'
        if movements == 'win':
            ret_val = True
            return ret_val
        if movements is not None:
            self.AddLogInfo(switch, movements)
            movements = af.TranslateAutoMoveToBoard(movements)
            self.data_model.movement = movements
            command = self.board.DoAllMovements(movements,color,skip_bar)
            full_command = self.board.ApplyCommand(command)
            af.Log().log_info("Command:" + full_command)
            if color == 'w':
                if self.data_model.white_pieces == []:
                    ret_val = True
            else:
                if self.data_model.black_pieces == []:
                    ret_val = True

            self.data_model.RestoreState()
        else:
            print ("no moves found, skip...")
            self.board.SendCommandToRGB("JUST_TEXT","No Moves..","Skipping..",0,0,0,0)
        return ret_val

    def AddLogInfo(self, switch, movements):
        board_string = self.data_model.CreateGameStringForGNUBG2(switch)
        af.Log().log_info("board:" + board_string)
        print (f"dice: {self.data_model.dice} movement: {movements}")
        af.Log().log_info("dice:" + str(self.data_model.dice))
        af.Log().log_info("Movement:" + str(movements))
        
    def TestRGB(self):
        while (True):
            d1 = random.randint(1, 6)
            d2 = random.randint(1, 6)
            self.board.SendCommandToRGB(self.ChooseMode(),"WHITE",self.ChooseTextForPlayer(),0,0,d1,d2)
            time.sleep(7)

    def HandleChar(self,char_input):
            if char_input & 0xFF == ord('n'):
                self.data_model.max_subcommand +=1
                split_commands = self.data_model.command.split('^')
                if self.data_model.max_subcommand >= len(split_commands):
                    self.data_model.max_subcommand = -1
            if char_input & 0xFF == ord('x'):
                self.ClearCommand()
            if char_input & 0xFF == ord('s'):
                #self.data_model.command = "%Move:G0 X201.40 Y41.89%Magnet:2:64:X:172.96%^Magnet:1:74^MagnetOn^Magnet:1:13^Move:G0 X59.53 Y8.07^Magnet:1:64^Move:G0 X48.69 Y4.07 F400^MagnetOff^Magnet:2:13^Move:G0 X134.12 Y78.93^Magnet:2:64^Move:G0 X136.15 Y78.93 F400^Magnet:1:74^MagnetOn^Magnet:1:13^Move:G0 X59.82 Y26.40^Magnet:1:64^Move:G0 X48.98 Y22.40 F400^MagnetOff^Magnet:2:13^Move:G0 X0.00 Y0.00"
                if (self.data_model.command != 0):    
                    self.board.SendCommand(self.data_model.command)
                    self.ClearCommand()
            if char_input & 0xFF == ord('r'):
                self.camera.AdjustRectangles()
            if char_input & 0xFF == ord('3'):    
                self.board.ArrangeCalibrationCheckersOnBoard()
            if char_input & 0xFF == ord('b'):
                self.camera.CalibrateBoard()
            if char_input & 0xFF == ord('6'):
                self.TestRGB()
            if char_input & 0xFF == ord('7'):
                #self.RollDice()
                self.GenerateDiceImages()

            if char_input & 0xFF == ord('j'):
                af.TakeSnapshot()
                self.data_model.SaveState()
                
            """if char_input & 0xFF == ord('.'):
                self.camera.DistortImageY(0.002)
            if char_input & 0xFF == ord(','):
                self.camera.DistortImageY(-0.002)"""
            if char_input & 0xFF == ord('o'):
                self.board.MoveToOrigin()

            if char_input & 0xFF == ord('y'):
                self.board.HandleMagnetServoPos(-3)
            if char_input & 0xFF == ord('h'):
                self.board.HandleMagnetServoPos(3)

            if char_input & 0xFF == ord('i'):
                self.board.HandleMagnetServoPos(-1)
            if char_input & 0xFF == ord('k'):
                self.board.HandleMagnetServoPos(1)

            if char_input & 0xFF == ord('q'):
                self.board.ToggleMagnetState()

            if char_input & 0xFF == ord('['):
                self.camera.DistortImageX(0.002)
            if char_input & 0xFF == ord(']'):
                self.camera.DistortImageX(-0.002)

            if char_input & 0xFF == ord('a'):
                self.data_model.dice = self.RollDiceUntilValid("I'm Rolling")
                self.SimulateAutoPlayer('w',True)

            if char_input & 0xFF == ord('9'):
                self.camera.dice_radius+=1
            if char_input & 0xFF == ord('0'):
                self.camera.dice_radius-=1

            if char_input & 0xFF == ord('1'):
                self.cur_state = self.STATE_AUTO_PLAYER
            if char_input & 0xFF == ord('2'):
                self.cur_state = self.STATE_PLAYER
            if char_input & 0xFF == ord('4'):
                self.TestRGB()
                


            """if char_input & 0xFF == ord('4'):
                self.camera.checkers_detection.HoughCircles_param2 -= 0.5
                print (f"param2 =  {self.camera.checkers_detection.HoughCircles_param2}")
            if char_input & 0xFF == ord('5'):
                self.camera.checkers_detection.HoughCircles_param2 += 0.5
                print (f"param2 =  {self.camera.checkers_detection.HoughCircles_param2}")

            if char_input & 0xFF == ord('6'):
                self.camera.checkers_detection.HoughCircles_param1 -= 1
                print (f"param1 =  {self.camera.checkers_detection.HoughCircles_param1}")
            if char_input & 0xFF == ord('7'):
                self.camera.checkers_detection.HoughCircles_param1 += 1
                print (f"param1 =  {self.camera.checkers_detection.HoughCircles_param1}")"""

                
            if char_input & 0xFF == ord('c'):
                self.data_model.demo_mode = True
                return True
            if char_input & 0xFF == ord('f'):
                self.data_model.demo_mode = False
                return True

            if char_input & 0xFF == ord('p'):
                self.camera.MoveMagnetAway()
            if char_input & 0xFF == ord('t'):
                self.camera.MoveMagnetToTest()

            if char_input & 0xFF == ord('u'):
                self.board.MagnetUp()
            if char_input & 0xFF == ord('d'):
                self.board.MagnetDown()
            if char_input & 0xFF == ord('z'):
                self.board.MagnetBar()
            if char_input & 0xFF == ord('m'):
                self.board.MagnetManeuver()
            """ if char_input & 0xFF == ord('x'):
                self.board.MagnetHover()"""

            return False

    def ClearCommand(self):
        self.screen.generated_board.ClearCommand()
        self.screen.data_model.dice = 0
        self.screen.data_model.movement = []
        self.screen.data_model.command = 0
        self.screen.data_model.max_subcommand = -1

    def SendStartGameDice(self,player_dice1,player_dice2,auto_dice1,auto_dice2):
        self.board.SendCommandToRGB("START_DICE",str(player_dice1),str(auto_dice1),player_dice2,auto_dice2,0,0)
    
    def GenerateDiceImages(self):

        #model_path = 'dice_recognizer_model.pth'  # Path to your trained model
        model_path = self.data_model.GetDiceModelName()

        # Load the model
        loaded_model = load_model(model_path)

        while (True):
            self.RollDice()
            time.sleep(0.5)
            dice_image = self.camera.TakeDiceSnapshot()
            filename = "C:\\temp\dice_image.jpg"
            cv2.imwrite(filename, dice_image)
            dice_predictions = predict_label(loaded_model,filename)
            existing_text = str(dice_predictions[0]) + ":" + str(dice_predictions[1])
            cv2.putText(dice_image, existing_text, (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1, cv2.LINE_AA)
            cv2.imshow("predictions", dice_image) 
            cv2.waitKey(2)
            #filename = "C:\\dice\\" + str(af.FindMissingNumber("c:\\dice","jpg"))+".jpg"
            #cv2.imwrite(filename, dice_image)

    def CheckValidImage(self):
        image = self.camera.CreateImageFromCamera()
        grey_value = af.mean_grey_value_from_camera(image)
        if grey_value < 190:    #threshold for image taken when camera is off
            return True
        return False

    def RunGame(self):
        self.InitGame()

        self.camera.InitCalibrate()
        self.screen.InitWindows()
        

        self.cur_state = self.STATE_DETECTION
        
        

        while (True):
            if self.cur_state == self.STATE_CALIBRATION:
                if (self.camera.Calibrate() == True):
                    self.cur_state= self.STATE_DETECTION
            if self.cur_state == self.STATE_DETECTION:
                self.WaitUntilImageIsValid()
                #self.camera.screen_mode = self.camera.MODE_GAME
                #self.camera.UnregisterMouse()
                self.camera.GetBoardPlayers()
                self.screen.PrepareWindows()
                movement_columns = self.screen.generated_board.GetMovement()
                if movement_columns != 0 and movement_columns != [] and movement_columns[-1][1] != -1:
                    self.data_model.movement = movement_columns
                self.data_model.SaveState()
                commands = self.board.DoAllMovements(self.data_model.movement,"b",False)
                self.board.ApplyCommand(commands)
                self.data_model.RestoreState()
                
                ret_val = self.screen.ShowWindows()
                if self.HandleChar(ret_val) == True:
                    if self.data_model.demo_mode == True:
                        self.cur_state = self.STATE_PLAYER # STATE_AUTO_PLAYER #
                    else:
                        self.cur_state = self.STATE_CHOOSE_PLAYER
            if self.cur_state== self.STATE_CHOOSE_PLAYER:
                no_one_starts = True
                player_dice,auto_dice = (0,0)
                self.SendStartGameDice(7,7,7,7)
                while no_one_starts:
                    self.board.WaitForPlayer(False)
                    player_dice = self.RollDice()
                    self.SendStartGameDice(player_dice[0],player_dice[1],7,7)
                    auto_dice = self.RollDice('')

                    if player_dice[0] + player_dice[1] > auto_dice[0] + auto_dice[1]:
                        self.cur_state = self.STATE_PLAYER
                        no_one_starts = False
                    if player_dice[0] + player_dice[1] < auto_dice[0] + auto_dice[1]:
                        self.cur_state = self.STATE_AUTO_PLAYER
                        no_one_starts = False

                    if self.cur_state in (self.STATE_AUTO_PLAYER,self.STATE_PLAYER):
                        self.SendStartGameDice(player_dice[0],player_dice[1],auto_dice[0],auto_dice[1])
                        time.sleep(3)
                        msg = "I start!" if self.cur_state == self.STATE_AUTO_PLAYER else "play!"
                        self.board.SendCommandToRGB("STARS",msg,"NONE",0,0,0,0)
                        self.board.WaitForPlayer(False)
                    else:
                        self.board.SendCommandToRGB("JUST_TEXT","Again..","NONE",0,0,0,0)
                        
            if self.cur_state == self.STATE_PLAYER:
                print ("Player Turn: (B)")
                af.Log().log_info("Player: Black")
                dice = self.RollDiceUntilValid("You're Rolling!")
                #dice =  (1,2) 
                print(dice)
                win_game = False
                if self.data_model.demo_mode:
                    self.board.SendCommandToRGB(self.ChooseMode(),"BLACK",self.ChooseTextForPlayer(),0,0,dice[0],dice[1])
                    self.data_model.dice = dice
                    win_game = self.SimulateAutoPlayer('b',False)

                    if win_game == 'resign':
                        self.board.SendCommandToRGB("VICTORY","NONE","NONE",0,0,0,0)    #if player resigns - computer wins
                        while True:
                            time.sleep(1)
                            pass

                    if (self.data_model.command != 0 and self.data_model.command != ''):
                        self.board.SendCommand(self.data_model.command)
                        if self.data_model.use_dice == False:
                            self.RefreshBoard(30)
                else:
                    #self.board.SendCommandToRGB(self.ChooseMode(),"Your Turn",self.ChooseTextForPlayer(),0,0,dice[0],dice[1])
                    self.board.SendCommandToRGB(self.ChooseMode(),"Your Turn","Play!",0,0,dice[0],dice[1])
                    self.board.WaitForPlayer(False)
                    self.RefreshBoard(10)
                
                if self.data_model.black_pieces == [] or win_game:
                    if self.data_model.demo_mode:
                        self.board.SendCommandToRGB("VICTORY","NONE","NONE",0,0,0,0)
                    else:
                        self.board.SendCommandToRGB("LOST","NONE","NONE",0,0,0,0)
                    while True:
                        pass
                self.cur_state = self.STATE_AUTO_PLAYER

            if self.cur_state == self.STATE_AUTO_PLAYER:
                print ("Auto Player Turn (W)")
                af.Log().log_info("Player: white")
                self.data_model.dice = self.RollDiceUntilValid("You're Rolling!")
                #self.data_model.dice =  (4,6) # bug - resign!!!
                print (self.data_model.dice)
                if self.data_model.demo_mode:
                    self.board.SendCommandToRGB(self.ChooseMode(),"WHITE",self.ChooseTextForAutoPlayer(),0,0,self.data_model.dice[0],self.data_model.dice[1])
                else:    
                    #self.board.SendCommandToRGB(self.ChooseMode(),"My Turn!",self.ChooseTextForAutoPlayer(),0,0,self.data_model.dice[0],self.data_model.dice[1])
                    self.board.SendCommandToRGB(self.ChooseMode(),"My Turn!","I Play",0,0,self.data_model.dice[0],self.data_model.dice[1])
                win_game = self.SimulateAutoPlayer('w',False)
                if win_game == 'resign':    #computer has no moves, hence resigning
                    self.board.SendCommandToRGB("RESIGN","NONE","NONE",0,0,0,0)
                    while True:
                        time.sleep(1)
                        pass
                    
                if (self.data_model.command != 0 and self.data_model.command != ''):
                    self.board.SendCommand(self.data_model.command)
                    if self.data_model.demo_mode and self.data_model.use_dice == False:
                        self.RefreshBoard(30)
                if win_game:
                    self.board.SendCommandToRGB("VICTORY","NONE","NONE",0,0,0,0)
                    while True:
                        time.sleep(1)
                        pass
                    
                self.cur_state = self.STATE_PLAYER

    def RefreshBoard(self,num_of_refresh):
        for i in range(num_of_refresh):
            self.camera.GetBoardPlayers()
            self.screen.PrepareWindows()
            ret_val = self.screen.ShowWindows()
        
  
    
game = BackgammonGame()

game.RunGame()

#C:\Users\user\AppData\Local\Packages\PythonSoftwareFoundation.Python.3.11_qbz5n2kfra8p0\LocalCache\local-packages\Python311\Scripts