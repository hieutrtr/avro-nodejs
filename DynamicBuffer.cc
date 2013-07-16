#include "DynamicBuffer.hh"
namespace avronode {

  /**
   * [newBuffer description]
   * @return [description]
   */
  DynamicBuffer::DynamicBuffer() {
    totalLength = 0;
    startPoint = 0;
    pFirstBlock = NULL;
    pLastBlock = pFirstBlock;
    pthread_mutex_init(&lock, NULL);
  }
  /**
   * [freeBuffer description]
   * @param pBuffer [pointer to the buffer to be freed]
   */
  DynamicBuffer::~DynamicBuffer() {
    /*there is more memory block in the linked list*/
    BufferBlock *block = pFirstBlock;
    while (block) {
      pFirstBlock = block->pNextBlock;
      free(block);
      block = pFirstBlock;
    }
    //free(pBuffer);
    //free the Buffer structure;
    return;
  }
  /**
   * [appendData description]
   * @param  pBuffer    [pointer to the abstract buffer]
   * @param  pInput     [pointer to the data source must be null terminated]
   * @param  offset     [offset of the input data]
   * @param  dataLength [number of bytes of the input data]
   * @return            [description]
   */
  long int DynamicBuffer::appendData(uint8_t *pInput, long int offset,    
               long int dataLength) {
    //there is more input data
    pthread_mutex_lock( &lock);
    BufferBlock *block = (BufferBlock*) malloc(sizeof(BufferBlock));
    
    block->data = pInput;
    block->size = dataLength;
    block->pNextBlock = NULL;

    if(pFirstBlock == NULL){
      pFirstBlock = block;
      pLastBlock = block;
    }else if(pFirstBlock != NULL){
      pLastBlock->pNextBlock = block;
      pLastBlock = block;
    }
    totalLength += dataLength;
    pthread_mutex_unlock( &lock);

    
    return totalLength;
  }
  /**
   * [readData description]
   * @param  pOutput     [pointer to the output byte array]
   * @param  offset      [offset of the output byte array]
   * @param  arrayLength [size of available output byte array]
   * @return             [description]
   */
  long int DynamicBuffer::readData(uint8_t **pOutput,
             long int offset,
             long int arrayLength) {
    pthread_mutex_lock( &lock);
    // while there is something more to read and there is room for output
    BufferBlock *block = pFirstBlock;
    
    uint8_t *byte = '\0';
   
    if( block) {

      byte = &(block->data[startPoint++]);
      totalLength--;
      //read from the first memory block;
      // if the first memory block is empty
      (pOutput[offset++]) = byte;
      if (block->size == startPoint) {
        //delete the first memory data[startPoint]block from the linked list and free its memory;
        pFirstBlock = block->pNextBlock;
        free(block);
        block = pFirstBlock;    
        startPoint = 0;
        pthread_mutex_unlock( &lock);

        return startPoint;
      }
    }
    pthread_mutex_unlock( &lock);

    return startPoint;
  }

  /**
   * [freeFirstBlock description]
   * @param pBuffer [description]
   */
  void DynamicBuffer::freeFirstBlock(){
    BufferBlock *block = pFirstBlock;
    pFirstBlock = block->pNextBlock;
    //free(block);
  }

}






