/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkDemonsRegistrationFunctionM.h,v $
  Language:  C++
  Date:      $Date: 2008-12-08 16:00:52 $
  Version:   $Revision: 1.29 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkDemonsRegistrationFunctionM_h
#define __itkDemonsRegistrationFunctionM_h

#include "itkPDEDeformableRegistrationFunctionM.h"
#include "itkPoint.h"
#include "itkCovariantVector.h"
#include "itkInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkCentralDifferenceImageFunction.h"

namespace itk {

/**
 * \class DemonsRegistrationFunctionM
 *
 * This class encapsulate the PDE which drives the demons registration
 * algorithm. It is used by DemonsRegistrationFilter to compute the
 * output deformation field which will map a moving image onto a
 * a fixed image.
 *
 * Non-integer moving image values are obtained by using
 * interpolation. The default interpolator is of type
 * LinearInterpolateImageFunction. The user may set other
 * interpolators via method SetMovingImageInterpolator. Note that the input
 * interpolator must derive from baseclass InterpolateImageFunction.
 *
 * This class is templated over the fixed image type, moving image type,
 * and the deformation field type.
 *
 * \warning This filter assumes that the fixed image type, moving image type
 * and deformation field type all have the same number of dimensions.
 *
 * \sa DemonsRegistrationFilter
 * \ingroup FiniteDifferenceFunctions
 */
template<class TFixedImage, class TMovingImage, class TDeformationField,class TMask=TFixedImage>
class ITK_EXPORT DemonsRegistrationFunctionM :
    public PDEDeformableRegistrationFunctionM< TFixedImage,TMovingImage,TDeformationField,TMask>
{
public:
  /** Standard class typedefs. */
  typedef DemonsRegistrationFunctionM    Self;
  typedef PDEDeformableRegistrationFunctionM< TFixedImage,
                                             TMovingImage, TDeformationField, TMask>
                                             Superclass;
					     
  typedef SmartPointer<Self>            Pointer;
  typedef SmartPointer<const Self>      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro( DemonsRegistrationFunctionM,
                PDEDeformableRegistrationFunctionM );

  /** MovingImage image type. */
  typedef typename Superclass::MovingImageType     MovingImageType;
  typedef typename Superclass::MovingImagePointer  MovingImagePointer;

  /** FixedImage image type. */
  typedef typename Superclass::FixedImageType     FixedImageType;
  typedef typename Superclass::FixedImagePointer  FixedImagePointer;
  typedef typename FixedImageType::IndexType      IndexType;
  typedef typename FixedImageType::SizeType       SizeType;
  typedef typename FixedImageType::SpacingType    SpacingType;
  
  /** MaskImage image type. */
  typedef typename Superclass::MovingImageType     MaskImageType;
  typedef typename Superclass::MovingImagePointer  MaskImagePointer;
  typedef typename MaskImageType::PixelType        MaskPixelType;
  

  /** Deformation field type. */
  typedef typename Superclass::DeformationFieldType  DeformationFieldType;
  typedef typename Superclass::DeformationFieldTypePointer
                                                     DeformationFieldTypePointer;

  /** Inherit some enums from the superclass. */
  itkStaticConstMacro(ImageDimension, unsigned 
int,Superclass::ImageDimension);

  /** Inherit some enums from the superclass. */
  typedef typename Superclass::PixelType         PixelType;
  typedef typename Superclass::RadiusType        RadiusType;
  typedef typename Superclass::NeighborhoodType  NeighborhoodType;
  typedef typename Superclass::FloatOffsetType   FloatOffsetType;
  typedef typename Superclass::TimeStepType      TimeStepType;

  /** Interpolator type. */
  typedef double                                     CoordRepType;
  typedef InterpolateImageFunction<MovingImageType,CoordRepType> 
                                                     InterpolatorType;
  typedef typename InterpolatorType::Pointer         InterpolatorPointer;
  typedef typename InterpolatorType::PointType       PointType;
  typedef LinearInterpolateImageFunction<MovingImageType,CoordRepType>
                                                     DefaultInterpolatorType;

  typedef InterpolateImageFunction<
    MaskImageType,CoordRepType>                     MaskInterpolatorType;
  typedef typename MaskInterpolatorType::Pointer    MaskInterpolatorPointer;
	
  typedef NearestNeighborInterpolateImageFunction<
    MaskImageType,CoordRepType>                     DefaultMaskInterpolatorType;

  /** Covariant vector type. */
  typedef CovariantVector<double,itkGetStaticConstMacro(ImageDimension)> 
CovariantVectorType;

  /** Fixed image gradient calculator type. */
  typedef CentralDifferenceImageFunction<FixedImageType> 
                                                    GradientCalculatorType;
  typedef typename GradientCalculatorType::Pointer   
                                                    GradientCalculatorPointer;

 /** Moving image gradient calculator type. */
  typedef CentralDifferenceImageFunction<MovingImageType,CoordRepType>
                                          MovingImageGradientCalculatorType;
  typedef typename MovingImageGradientCalculatorType::Pointer
                                          MovingImageGradientCalculatorPointer;

  /** Set the moving image interpolator. */
  void SetMovingImageInterpolator( InterpolatorType * ptr )
  { m_MovingImageInterpolator = ptr; }

  /** Get the moving image interpolator. */
  InterpolatorType * GetMovingImageInterpolator(void)
  { return m_MovingImageInterpolator; }
  
  /** Set the moving image interpolator. */
  void SetMovingImageMaskInterpolator( MaskInterpolatorType * ptr )
    { m_MovingImageMaskInterpolator = ptr; }

  /** Get the moving image interpolator. */
  InterpolatorType * GetMovingImageMaskInterpolator(void)
    { return m_MovingImageMaskInterpolator; }

  /** This class uses a constant timestep of 1. */
  virtual TimeStepType ComputeGlobalTimeStep(void * itkNotUsed(GlobalData)) 
const
    { return m_TimeStep; }

  /** Return a pointer to a global data structure that is passed to
   * this object from the solver at each calculation.  */
  virtual void *GetGlobalDataPointer() const
    {
    GlobalDataStruct *global = new GlobalDataStruct();
    global->m_SumOfSquaredDifference  = 0.0;
    global->m_NumberOfPixelsProcessed = 0L;
    global->m_SumOfSquaredChange      = 0;
    return global;
    }

  /** Release memory for global data structure. */
  virtual void ReleaseGlobalDataPointer( void *GlobalData ) const;

  /** Set the object's state before each iteration. */
  virtual void InitializeIteration();

  /** This method is called by a finite difference solver image filter at
   * each pixel that does not lie on a data set boundary */
  virtual PixelType  ComputeUpdate(const NeighborhoodType &neighborhood,
                                   void *globalData,
                                   const FloatOffsetType &offset = 
FloatOffsetType(0.0));

  /** Get the metric value. The metric value is the mean square difference 
   * in intensity between the fixed image and transforming moving image 
   * computed over the the overlapping region between the two images. */
  virtual double GetMetric() const
    { return m_Metric; }

  /** Get the rms change in deformation field. */
  virtual double GetRMSChange() const
    { return m_RMSChange; }

  /** Select if the fixed image or moving image gradient is used for
   * the computating the demon forces. The fixed image gradient is used
   * by default. */
  virtual void SetUseMovingImageGradient( bool flag )
    { m_UseMovingImageGradient = flag; }
  virtual bool GetUseMovingImageGradient() const
    { return m_UseMovingImageGradient; }

  /** Set/Get the threshold below which the absolute difference of
   * intensity yields a match. When the intensities match between a
   * moving and fixed image pixel, the update vector (for that
   * iteration) will be the zero vector. Default is 0.001. */
  virtual void SetIntensityDifferenceThreshold(double);
  virtual double GetIntensityDifferenceThreshold() const;
  
protected:
  DemonsRegistrationFunctionM();
  ~DemonsRegistrationFunctionM() {}
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** FixedImage image neighborhood iterator type. */
  typedef ConstNeighborhoodIterator<FixedImageType> FixedImageNeighborhoodIteratorType;

  /** A global data type for this class of equation. Used to store
   * information for computing the metric. */
  struct GlobalDataStruct
    {
    double          m_SumOfSquaredDifference;
    unsigned long   m_NumberOfPixelsProcessed;
    double          m_SumOfSquaredChange;
    };

private:
  DemonsRegistrationFunctionM(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  /** Cache fixed image information. */
  //SpacingType                  m_FixedImageSpacing;
  //PointType                    m_FixedImageOrigin;
  PixelType                       m_ZeroUpdateReturn;
  double                          m_Normalizer;

  /** Function to compute derivatives of the fixed image. */
  GradientCalculatorPointer       m_FixedImageGradientCalculator;

  /** Function to compute derivatives of the moving image. */
  MovingImageGradientCalculatorPointer       m_MovingImageGradientCalculator;
  bool                                       m_UseMovingImageGradient;

  /** Function to interpolate the moving image. */
  InterpolatorPointer             m_MovingImageInterpolator;
  
  /** Function to interpolate the moving image mask */
  MaskInterpolatorPointer         m_MovingImageMaskInterpolator;

  /** The global timestep. */
  TimeStepType                    m_TimeStep;

  /** Threshold below which the denominator term is considered zero. */
  double                          m_DenominatorThreshold;

  /** Threshold below which two intensity value are assumed to match. */
  double                          m_IntensityDifferenceThreshold;

  /** The metric value is the mean square difference in intensity between
   * the fixed image and transforming moving image computed over the 
   * the overlapping region between the two images. */
  mutable double                  m_Metric;
  mutable double                  m_SumOfSquaredDifference;
  mutable unsigned long           m_NumberOfPixelsProcessed;
  mutable double                  m_RMSChange;
  mutable double                  m_SumOfSquaredChange;

  /** Mutex lock to protect modification to metric. */
  mutable SimpleFastMutexLock     m_MetricCalculationLock;

};


} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDemonsRegistrationFunctionM.txx"
#endif

#endif
