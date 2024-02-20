// Rotary encoder example
 const int DT = 2;
 const int CLK = 3;

 int counter = 0; 
 int State;
 int LastState;  

void setup() { 
  pinMode (DT,INPUT);
  pinMode (CLK,INPUT);
  Serial.begin (9600);
  LastState = digitalRead(CLK);   
} 

void loop() { 
  State = digitalRead(CLK); // Read CLK
  // If CLK changed, that means a Pulse has occurred
  if ((State != LastState) && State){     
    // If DT is different => clockwise
    if (digitalRead(DT) != State) { 
      counter ++;
    } else {
      counter --;
    }
    Serial.print("Position: ");
    Serial.println(counter);
  } 
  LastState = State; // Remember CLK
}
