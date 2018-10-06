/** 
 * @file usb/audio.h
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __USB_AUDIO_H__
#define __USB_AUDIO_H__

#include <stdint.h>

/* Descriptor gives detail about the whole device.*/
#define AUDIO_DESCRIPORT_DEVICE                     0x21
/* Descriptor gives detail about a configuration.*/
#define AUDIO_DESCRIPORT_CONFIGURATION              0x22
/* Descriptor gives detail about a string.*/
#define AUDIO_DESCRIPORT_STRING                     0x23
/* Descriptor gives detail about an interface.*/
#define AUDIO_Descriptor_INTERFACE                  0x24
/* Descriptor gives detail about an endpoint.  */
#define AUDIO_Descriptor_ENDPOINT                   0x25

/* Header descriptor subtype.*/
#define AUDIO_DESCRIPTOR_HEADER                     0x01
/* Input terminal descriptor subtype.*/
#define AUDIO_DESCRIPTOR_INPUTTERMINAL              0x02
/* Output terminal descriptor subtype.*/
#define AUDIO_DESCRIPTOR_OUTPUTTERMINAL             0x03
/* Mixer unit descriptor subtype.*/
#define AUDIO_DESCRIPTOR_MIXERUNIT                  0x04
/* Selector unit descriptor subtype.*/
#define AUDIO_DESCRIPTOR_SELECTORUNIT               0x05
/* Feature unit descriptor subtype.*/
#define AUDIO_DESCRIPTOR_FEATUREUNIT                0x06
/* Processing unit descriptor subtype.*/
#define AUDIO_DESCRIPTOR_PROCESSINGUNIT             0x07
/* Extension unit descriptor subtype.*/
#define AUDIO_DESCRIPTOR_EXTENSIONUNIT              0x08

/* Class code for a USB audio device. */
#define AUDIO_DESCRIPTOR_DEVICE_CLASS                       0x00
/* Subclass code for a USB audio device. */
#define AUDIO_DESCRIPTOR_DEVICE_SUBCLASS                    0x00
/* Protocol code for a USB audio device. */
#define AUDIO_DESCRIPTOR_DEVICE_PROTOCOL                    0x00

/* Class code for an USB audio streaming interface.*/
#define AUDIO_DESCRIPTOR_INTERFACE_STREAMING_CLASS               0x01
/* Subclass code for an audio streaming interface.*/
#define AUDIO_DESCRIPTOR_INTERFACE_STREAMING_SUBCLASS            0x02
/* Protocol code for an audio streaming interface.*/
#define AUDIO_DESCRIPTOR_INTERFACE_STREAMING_PROTOCOL            0x00

/* General descriptor subtype.*/
#define AUDIO_DESCRIPTOR_INTERFACE_STREAMING_GENERAL             0x01
/* Format type descriptor subtype.*/
#define AUDIO_DESCRIPTOR_INTERFACE_STREAMING_FORMATTYPE          0x02
/* Format specific descriptor subtype.*/
#define AUDIO_DESCRIPTOR_INTERFACE_STREAMING_FORMATSPECIFIC      0x03


/* Class code for an audio control interface. */
#define AUDIO_DESCRIPTOR_INTERFACE_CONTROL_CLASS             0x01
/* Subclass code for an audio control interface. */
#define AUDIO_DESCRIPTOR_INTERFACE_CONTROL_SUBCLASS          0x01
/* Protocol code for an audio control interface. */
#define AUDIO_DESCRIPTOR_INTERFACE_CONTROL_PROTOCOL          0x00

/* Descriptor subtype for an Audio data endpoint. */
#define AUDIO_DESCRIPTOR_ENDPOINT_DATA_SUBTYPE                   0x01

/* Lock delay is expressed in milliseconds. */
#define AUDIO_DESCRIPTOR_ENDPOINT_DATA_MILLISECONDS              1
/* Lock delay is expressed in decoded PCM samples. */
#define AUDIO_DESCRIPTOR_ENDPOINT_DATA_PCMSAMPLES                2

/* Identifies the USB audio specification release 1.00.*/
#define AUDIO_DESCRIPTOR_HEADER_AUD1_00                 0x0100

#define AUDIO_DESCRIPTOR_TERMINAL_USBSTREAMING          0x0101
#define AUDIO_DESCRIPTOR_TERMINAL_MICROPHONE            0x0201
#define AUDIO_DESCRIPTOR_TERMINAL_HANDSETIN             0x0401
#define AUDIO_DESCRIPTOR_TERMINAL_SPEAKERPHONE          0x0403
#define AUDIO_DESCRIPTOR_TERMINAL_LINEIN                0x0501

/* A terminal receiving its data from a USB isochronous endpoint.*/
#define AUDIO_DESCRIPTOR_TERMINAL_INPUT_USBSTREAMING         0x0101
/* A terminal sampling data from a microphone.*/
#define AUDIO_DESCRIPTOR_TERMINAL_INPUT_MICROPHONE           0x0201
/* A terminal sampling data from a Handset In.*/
#define AUDIO_DESCRIPTOR_TERMINAL_INPUT_HANDSETIN            0x0401
/* A terminal sampling data from a speaker-phone*/
#define AUDIO_DESCRIPTOR_TERMINAL_INPUT_SPEAKERPHONE         0x0403
/* A terminal sampling data from a Phone Line In.*/
#define AUDIO_DESCRIPTOR_TERMINAL_INPUT_LINEIN               0x0501

/* Front left channel.*/
#define AUDIO_DESCRIPTOR_TERMINAL_INPUT_LEFTFRONT            (1 << 0)
/* Front right channel.*/
#define AUDIO_DESCRIPTOR_TERMINAL_INPUT_RIGHTFRONT           (1 << 1)
/* Front center channel.*/
#define AUDIO_DESCRIPTOR_TERMINAL_INPUT_CENTERFRONT          (1 << 2)

/* A terminal sending data through USB isochronous endpoint.*/
#define AUDIO_DESCRIPTOR_TERMINAL_OUTPUT_USBTREAMING         0x0101
/* A terminal sending data to a USB host through an Isochronous endpoint.*/
#define AUDIO_DESCRIPTOR_TERMINAL_OUTPUT_SPEAKER             0x0301
/* A terminal sending data to Handset Out.*/
#define AUDIO_DESCRIPTOR_TERMINAL_OUTPUT_HANDSETOUT          0x0401
/* A terminal sending data to Phone Line Out.*/
#define AUDIO_DESCRIPTOR_TERMINAL_OUTPUT_LINEOUT             0x0501

/* Channel mute control. */
#define AUDIO_DESCRIPTOR_FEATURE_UNIT_MUTE               (1 << 0)
/* Channel volume control. */
#define AUDIO_DESCRIPTOR_FEATURE_UNIT_VOLUME             (1 << 1)
/* Channel bass control. */
#define AUDIO_DESCRIPTOR_FEATURE_UNIT_BASS               (1 << 2)
/* Channel middle control. */
#define AUDIO_DESCRIPTOR_FEATURE_UNIT_MID                (1 << 3)
/* Channel treble control. */
#define AUDIO_DESCRIPTOR_FEATURE_UNIT_TREBLE             (1 << 4)


/* Format type for a format type I descriptor. */
#define AUDIO_DESCRIPTOR_FORMAT_TYPE_I_FORMATTYPEONE        0x01

/* AUDIO_DESCRIPTOR_FORMAT_TYPE_I_PCM - PCM format. */
#define AUDIO_DESCRIPTOR_FORMAT_TYPE_I_PCM                  0x0001

/* Indicates the sampling frequency can have any value in the provided range. */
#define AUDIO_DESCRIPTOR_FORMAT_TYPE_I_CONTINUOUS           0


struct audio_descriptor_interface_streaming {
	/* Size of descriptor in bytes.*/
	uint8_t bLength;
	/* Descriptor type (AUDIO_DESCRIPTOR_INTERFACE).*/
	uint8_t bDescriptorType;
	/* Descriptor subtype (AUDIO_DESCRIPTOR_INTERFACE_STREAMING_GENERAL).*/
	uint8_t bDescriptorSubType;
	/* Terminal ID to which the interface is connected.*/
	uint8_t bTerminalLink;
	/* Delay introduced by the data path, in number of frames.*/
	uint8_t bDelay;
	/* Audio data format used by this interface.*/
	uint16_t wFormatTag;
} __attribute__((__packed__));

struct AUDIO_DESCRIPTOR_ENDPOINT {
   /* Size of the descriptor in bytes. */
   uint8_t bLength;
   /* Descriptor type (USBGenericDescriptor_ENDPOINT). */
   uint8_t bDescriptorType;
   /* Address and direction of the endpoint. */
   uint8_t bEndpointAddress;
   /* Endpoint type and additional characteristics (for isochronous endpoints). */
   uint8_t bmAttributes;
   /* Maximum packet size (in bytes) of the endpoint. */
   uint16_t wMaxPacketSize;
   /* Polling rate of the endpoint. */
   uint8_t bInterval;
   /* Refresh rate for a feedback endpoint. */
   uint8_t bRefresh;
   /* Address of the associated feedback endpoint if any. */
   uint8_t bSyncAddress;
} __attribute__((__packed__));

struct audio_descriptor_endpoint_data {
	/* Size of descriptor in bytes. */
	uint8_t bLength;
	/* Descriptor type
		( AUDIO_DESCRIPTOR_ENDPOINT). */
	uint8_t bDescriptorType;
	/* Descriptor subtype
		( AUDIO_DESCRIPTOR_ENDPOINT_DATA_SUBTYPE).  */
	uint8_t bDescriptorSubType;
	/* Indicates available controls and requirement on packet sizes. */
	uint8_t bmAttributes;
	/* Indicates the units of the wLockDelay fields.
		\sa usb_audio_lock_delay USB Audio Lock delay units */
	uint8_t bLockDelayUnits;
	/* Time it takes for the endpoint to lock its internal clock circuitry. */
	uint16_t wLockDelay;
} __attribute__((__packed__));

struct audio_descriptor_header {
	/* Size of descriptor in bytes.*/
	uint8_t bLength;
	/* Descriptor type ( AUDIO_DESCRIPTOR_INTERFACE).*/
	uint8_t bDescriptorType;
	/* Descriptor subtype ( AUDIO_DESCRIPTOR_HEADER).*/
	uint8_t bDescriptorSubType;
	/* Audio class release number in BCD format
	 * \sa usb_audio_class_ver USB Audio class releases */
	uint16_t bcdADC;
	/* Length of all descriptors used to qualify the Audio Control interface.*/
	uint16_t wTotalLength;
	/* Number of Streaming interfaces contained in this collection.*/
	uint8_t bInCollection;
} __attribute__((__packed__));

struct audio_descriptor_terminal_input {
	/* Size of descriptor in bytes.*/
	uint8_t bLength;
	/* Descriptor type (AUDIO_DESCRIPTOR_INTERFACE).*/
	uint8_t bDescriptorType;
	/* Descriptor subtype (AUDIO_DESCRIPTOR_INPUTTERMINAL).*/
	uint8_t bDescriptorSubType;
	/* ID of the terminal in the audio function.*/
	uint8_t bTerminalID;
	/* Terminal type.
	 * \sa usb_audio_in_term USB Audio Input terminal types
	 */
	uint16_t wTerminalType;
	/* ID of the output terminal to which this input terminal is associated.*/
	uint8_t bAssocTerminal;
	/* Number of logical output channels in this terminal.*/
	uint8_t bNrChannels;
	/* Spatial configuration of the logical channels.*/
	uint16_t wChannelConfig;
	/* Index of a string descriptor for the first logical channel.*/
	uint8_t iChannelNames;
	/* Index of a string descriptor for this terminal. */
	uint8_t iTerminal;
} __attribute__((__packed__));

struct audio_descriptor_terminal_output {
	/* Size of descriptor in bytes.*/
	uint8_t bLength;
	/* Descriptor type ( AUDIO_DESCRIPTOR_INTERFACE).*/
	uint8_t bDescriptorType;
	/* Descriptor subtype ( AUDIO_DESCRIPTOR_OUTPUTTERMINAL).  */
	uint8_t bDescriptorSubType;
	/* Identifier for this terminal.*/
	uint8_t bTerminalID;
	/* Terminal type.
	 *  \sa "USB Audio Output terminal types" */
	uint16_t wTerminalType;
	/* Identifier of the associated input terminal.*/
	uint8_t bAssocTerminal;
	/* Identifier of the unit or terminal to which this terminal
	 *  is connected.*/
	uint8_t bSourceID;
	/* Index of a string descriptor for this terminal.*/
	uint8_t iTerminal;
} __attribute__((__packed__));

struct audio_descriptor_feature_unit {
	/* Size of descriptor in bytes.*/
	uint8_t bLength;
	/* Descriptor type (AUDIO_DESCRIPTOR_INTERFACE). */
	uint8_t bDescriptorType;
	/* Descriptor subtype (AUDIO_DESCRIPTOR_FEATURE). */
	uint8_t bDescriptorSubType;
	/* Identifier of this feature unit. */
	uint8_t bUnitID;
	/* Identifier of the unit or terminal this feature unit is connected to. */
	uint8_t bSourceID;
	/* Size in bytes of a channel controls field. */
	uint8_t bControlSize;
} __attribute__((__packed__));

struct audio_descriptor_format_type_i {
	/* Size of descriptor in bytes. */
	uint8_t bLength;
	/* Descriptor type ( AUDIO_DESCRIPTOR_INTERFACE). */
	uint8_t bDescriptorType;
	/* Descriptor subtype ( AUDIO_DESCRIPTOR_INTERFACE_STREAMING_FORMATTYPE).*/
	uint8_t bDescriptorSubType;
	/* Format type ( AUDIO_DESCRIPTOR_FORMAT_TYPE_I_FORMATTYPEONE).*/
	uint8_t bFormatType;
	/* Number of physical channels in the audio stream.*/
	uint8_t bNrChannels;
	/* Number of bytes occupied by one audio subframe.*/
	uint8_t bSubFrameSize;
	/* Number of bits effectively used in an audio subframe.*/
	uint8_t bBitResolution;
	/* Number of supported discrete sampling frequencies, or
	    AUDIO_DESCRIPTOR_FORMAT_TYPE_I_CONTINUOUS.*/
	uint8_t bSamFreqType;
} __attribute__((__packed__));



#define AUDIO_OUT_EP                                  0x01
#define USB_AUDIO_CONFIG_DESC_SIZ                     109
#define AUDIO_INTERFACE_DESC_SIZE                     9
#define USB_AUDIO_DESC_SIZ                            0x09
#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE             0x09
#define AUDIO_STREAMING_ENDPOINT_DESC_SIZE            0x07

#define AUDIO_DESCRIPTOR_TYPE                         0x21
#define USB_DEVICE_CLASS_AUDIO                        0x01
#define AUDIO_SUBCLASS_AUDIOCONTROL                   0x01
#define AUDIO_SUBCLASS_AUDIOSTREAMING                 0x02
#define AUDIO_PROTOCOL_UNDEFINED                      0x00
#define AUDIO_STREAMING_GENERAL                       0x01
#define AUDIO_STREAMING_FORMAT_TYPE                   0x02

/* Audio Descriptor Types */
#define AUDIO_INTERFACE_DESCRIPTOR_TYPE               0x24
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE                0x25

/* Audio Control Interface Descriptor Subtypes */
#define AUDIO_CONTROL_HEADER                          0x01
#define AUDIO_CONTROL_INPUT_TERMINAL                  0x02
#define AUDIO_CONTROL_OUTPUT_TERMINAL                 0x03
#define AUDIO_CONTROL_FEATURE_UNIT                    0x06

#define AUDIO_INPUT_TERMINAL_DESC_SIZE                0x0C
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE               0x09
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE           0x07

#define AUDIO_CONTROL_MUTE                            0x0001

#define AUDIO_FORMAT_TYPE_I                           0x01
#define AUDIO_FORMAT_TYPE_III                         0x03

#define AUDIO_ENDPOINT_GENERAL                        0x01

#define AUDIO_REQ_GET_CUR                             0x81
#define AUDIO_REQ_SET_CUR                             0x01

#define AUDIO_OUT_STREAMING_CTRL                      0x02


















#endif /* __USB_AUDIO_H__ */
