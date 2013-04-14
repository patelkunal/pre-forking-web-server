RM := rm -rf
MAIN_FILE_NAME := main
TARGET := $(MAIN_FILE_NAME)
OBJS := $(MAIN_FILE_NAME).o
SRCS := $(MAIN_FILE_NAME).c

all : $(TARGET)

$(TARGET): $(OBJS) $(SRCS)
  @echo 'Building ' $(TARGET)
	@gcc -o $(TARGET) $(OBJS)
	@echo 'Build Successful'
	
%.o: %.c
	@echo 'Building $@ from $<'
	@gcc -o $@ -c $<
	
clean:
	$(RM) $(OBJS) $(TARGET) a.out
	@echo 'Clean successful'
